#include "frame.h"

Mat Frame::getRawFrame() const
{
    return rawFrame;
}

void Frame::setRawFrame(const Mat &value)
{
    rawFrame = value;
    convert2QImage();
}

QImage Frame::getFrame() const
{
    return frame;
}

void Frame::setFrame(const QImage &value)
{
    frame = value;
    Frame::update();
    emit(frameChanged());
}

bool Frame::smoothImage()
{
    if(!rawFrame.empty())
    {
        Mat temp;
        GaussianBlur( rawFrame, temp, Size( 31, 31 ), 0, 0 );
        setRawFrame(temp);
        return true;
    }
    return false;
}

bool Frame::erodeImage()
{
    if(!rawFrame.empty()){
        Mat temp;
        Mat kernel = getStructuringElement(MORPH_RECT, Size(16,16));
        erode(rawFrame, temp, kernel);
        setRawFrame(temp);
        return true;
    }
    return false;
}

bool Frame::dilateImage()
{
    if(!rawFrame.empty()){
        Mat temp = rawFrame;
        Mat kernel = getStructuringElement(MORPH_RECT, Size(6,6));
        dilate(temp, temp, kernel);
        setRawFrame(temp);
        return true;
    }
    return false;
}

void Frame::openImage(QString url)
{
    url.remove("file://");
    rawFrame = imread(url.toStdString());
    convert2QImage();
}

bool Frame::applyCanny()
{
    if(!rawFrame.empty())
    {
        Mat tempGray;
        cvtColor( rawFrame, tempGray, COLOR_BGR2GRAY );
        Canny( tempGray, tempGray, 0, 150, 3 );
        setRawFrame(tempGray);
        return true;
    }
    return false;
}

bool Frame::findHorizontalLines()
{
    if(!rawFrame.empty())
    {
        Mat temp;
        Mat binary;
        cvtColor(rawFrame, temp, COLOR_BGR2GRAY);
        adaptiveThreshold(temp, binary, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, -2);
        int structureSize = binary.cols / 30;

        Mat horizontalStructure = getStructuringElement(MORPH_RECT, Size(structureSize, 1));

        erode(binary, binary, horizontalStructure, Point(-1, -1));
        dilate(binary, binary, horizontalStructure, Point(-1, -1));

        setRawFrame(binary);
        return true;
    }
    return false;
}

bool Frame::findVerticalLines()
{
    if(!rawFrame.empty())
    {
        Mat temp;
        Mat binary;
        cvtColor(rawFrame, temp, COLOR_BGR2GRAY);
        adaptiveThreshold(temp, binary, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, -2);
        int structureSize = binary.rows / 30;

        Mat verticalStructure = getStructuringElement(MORPH_RECT, Size(1, structureSize));

        erode(binary, binary, verticalStructure, Point(-1, -1));
        dilate(binary, binary, verticalStructure, Point(-1, -1));

        setRawFrame(binary);
        return true;
    }
    return false;
}

bool Frame::laplacianFilter()
{
    if(!rawFrame.empty())
    {
        Mat temp, dst;
        cvtColor(rawFrame, temp, COLOR_BGR2GRAY);
        Laplacian( temp, dst, CV_16S, 3, 1, 0, BORDER_DEFAULT );
        convertScaleAbs(dst, dst);
        setRawFrame(dst);
        return true;
    }
    return false;
}

bool Frame::findImageContours()
{
    if(!rawFrame.empty())
    {
        Mat cannyTemp;

        cvtColor( rawFrame, cannyTemp, COLOR_BGR2GRAY );
        Canny( cannyTemp, cannyTemp, 0, 150, 3 );

        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        findContours(cannyTemp, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

        Mat drawing = Mat::zeros( cannyTemp.size(), CV_8UC3 );

        for( size_t i = 0; i< contours.size(); i++ )
        {
            Scalar color = Scalar( 256, 27, 120 );
            drawContours( drawing, contours, (int)i, color, 2, LINE_8, hierarchy, 0 );
        }
        setRawFrame(drawing);
        return true;
    }
    return false;
}

void Frame::convert2QImage()
{
    Mat tempMat;
    cvtColor(rawFrame, tempMat, COLOR_RGB2BGR);
    QImage tempImage((uchar*) tempMat.data, tempMat.cols,tempMat.rows, tempMat.step, QImage::Format_RGB888);

    frame = tempImage;
    frame.detach();
    Frame::update();
    emit(frameChanged());
}

Frame::Frame(QQuickItem *parent): QQuickPaintedItem(parent)
{

}

void Frame::paint(QPainter *painter)
{
    frame.scaled(640, 480, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    painter->drawImage(0, 0, frame, 0, 0, -1, -1, Qt::AutoColor);
}
