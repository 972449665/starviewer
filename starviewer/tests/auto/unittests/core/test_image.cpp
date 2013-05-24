#include "autotest.h"
#include "image.h"

#include "imageorientation.h"
#include "fuzzycomparetesthelper.h"
#include "series.h"
#include "mathtools.h"

#include <vtkImageData.h>

using namespace udg;
using namespace testing;

class test_Image : public QObject {
Q_OBJECT

private slots:
    void hasOverlays_ReturnExpectedValues_data();
    void hasOverlays_ReturnExpectedValues();

    void getDisplayShutterForDisplay_ShouldReturnExpectedValues_data();
    void getDisplayShutterForDisplay_ShouldReturnExpectedValues();

    void addWindowLevel_ShouldAddWindowLevel_data();
    void addWindowLevel_ShouldAddWindowLevel();
    void addWindowLevel_ShouldNotAddWindowLevel_data();
    void addWindowLevel_ShouldNotAddWindowLevel();
    void addWindowLevel_ShouldAddSameWindowLevelTwoTimes();
    
    void setWindowLevelList_SetsValuesAsExpected_data();
    void setWindowLevelList_SetsValuesAsExpected();
    
    void getWindowLevel_ShouldReturnExpectedValues_data();
    void getWindowLevel_ShouldReturnExpectedValues();

    void getDisplayShutterForDisplayAsVtkImageData_ShouldReturnNull_data();
    void getDisplayShutterForDisplayAsVtkImageData_ShouldReturnNull();

    void getDisplayShutterForDisplayAsVtkImageData_ShouldReturnNullWhenThereAreNoShutters();
    
    void getDisplayShutterForDisplayAsVtkImageData_ShouldReturnExpectedValues_data();
    void getDisplayShutterForDisplayAsVtkImageData_ShouldReturnExpectedValues();

    void distance_ReturnsExpectedValues_data();
    void distance_ReturnsExpectedValues();

    void getPreferredPixelSpacing_ReturnsExpectedSpacing_data();
    void getPreferredPixelSpacing_ReturnsExpectedSpacing();
};

Q_DECLARE_METATYPE(QList<DisplayShutter>)
Q_DECLARE_METATYPE(DisplayShutter)
Q_DECLARE_METATYPE(double*)
Q_DECLARE_METATYPE(Image*)
Q_DECLARE_METATYPE(WindowLevel)
Q_DECLARE_METATYPE(QList<WindowLevel>)
Q_DECLARE_METATYPE(PixelSpacing2D)

void test_Image::hasOverlays_ReturnExpectedValues_data()
{
    QTest::addColumn<unsigned short>("numberOfOverlays");
    QTest::addColumn<bool>("hasOverlays");
    unsigned short ushortZero = 0;
    QTest::newRow("Zero overlays") << ushortZero << false;
    for (unsigned short i = 1; i < 17; ++i)
    {
        QTest::newRow(qPrintable(QString("%1 overlays").arg(i))) << i << true;
    }
}

void test_Image::hasOverlays_ReturnExpectedValues()
{
    QFETCH(unsigned short, numberOfOverlays);
    QFETCH(bool, hasOverlays);

    Image image;
    QVERIFY(!image.hasOverlays());

    image.setNumberOfOverlays(numberOfOverlays);
    
    QCOMPARE(image.hasOverlays(), hasOverlays);
}

void test_Image::getDisplayShutterForDisplay_ShouldReturnExpectedValues_data()
{
    QTest::addColumn<int>("rows");
    QTest::addColumn<int>("columns");
    QTest::addColumn<QList<DisplayShutter> >("imageShuttersList");
    QTest::addColumn<DisplayShutter>("expectedShutterForDisplay");

    DisplayShutter rectangularShutter;
    rectangularShutter.setPoints(QPoint(1, 1), QPoint(10, 10)); 
    
    QTest::newRow("Image with no shutters") << 10 << 10 << QList<DisplayShutter>() << DisplayShutter();

    QList<DisplayShutter> shuttersList;
    
    shuttersList.clear();
    shuttersList << rectangularShutter;
    QTest::newRow("1 shutter, rectangular shape, same size as image") << 10 << 10 << shuttersList << DisplayShutter();
    
    shuttersList.clear();
    shuttersList << DisplayShutter();
    QTest::newRow("1 shutter, undefined shape") << 10 << 10 << shuttersList << DisplayShutter();

    shuttersList.clear();
    shuttersList << rectangularShutter;
    QTest::newRow("1 shutter, rectangular shape, smaller than image's size") << 15 << 15 << shuttersList << rectangularShutter;

    DisplayShutter rectangularShutter2;
    rectangularShutter2.setPoints(QPoint(-5, -5), QPoint(20, 20));
    
    shuttersList.clear();
    shuttersList << rectangularShutter2;
    QTest::newRow("1 shutter, rectangular shape, larger than image's size") << 10 << 10 << shuttersList << DisplayShutter();
    
    DisplayShutter rectangularShutter3;
    rectangularShutter3.setPoints(QPoint(5, 5), QPoint(20, 20));

    DisplayShutter intersectionShutter;
    QPolygon intersectionPolygon; 
    intersectionPolygon << QPoint(10, 5) << QPoint(10, 10) << QPoint(5, 10) << QPoint(5, 5) << QPoint(10, 5);
    intersectionShutter.setPoints(intersectionPolygon);
    
    shuttersList.clear();
    shuttersList << rectangularShutter;
    shuttersList << rectangularShutter3;
    QTest::newRow("2 shutters, intersection gives rectangular shape, smaller than image's size") << 15 << 15 << shuttersList << intersectionShutter;

    DisplayShutter circularShutter;
    circularShutter.setPoints(QPoint(5,5), 2);
    
    shuttersList.clear();
    shuttersList << DisplayShutter();
    shuttersList << circularShutter;
    shuttersList << rectangularShutter;
    QTest::newRow("3 shutters, 2 no appliable (empty and same image size rectangle)") << 10 << 10 << shuttersList << circularShutter;
}

void test_Image::getDisplayShutterForDisplay_ShouldReturnExpectedValues()
{
    QFETCH(int, rows);
    QFETCH(int, columns);
    QFETCH(QList<DisplayShutter>, imageShuttersList);
    QFETCH(DisplayShutter, expectedShutterForDisplay);

    Image *image = new Image;
    image->setRows(rows);
    image->setColumns(columns);
    image->setDisplayShutters(imageShuttersList);

    DisplayShutter shutterForDisplay = image->getDisplayShutterForDisplay();

    QCOMPARE(shutterForDisplay.getShape(), expectedShutterForDisplay.getShape());
    QCOMPARE(shutterForDisplay.getAsQPolygon(), expectedShutterForDisplay.getAsQPolygon());
}

void test_Image::addWindowLevel_ShouldAddWindowLevel_data()
{
    QTest::addColumn<WindowLevel>("windowLevel");
    
    QTest::newRow("both positive") << WindowLevel(100.0, 200.0);
    QTest::newRow("both negative") << WindowLevel(-101.0, -201.0);
    QTest::newRow("window positive, level negative") << WindowLevel(102.0, -202.0);
    QTest::newRow("window negative, level positive") << WindowLevel(-103.0, 203.0);
    QTest::newRow("level zero") << WindowLevel(100.0, 0.0);
}

void test_Image::addWindowLevel_ShouldAddWindowLevel()
{
    QFETCH(WindowLevel, windowLevel);

    Image image;
    image.addWindowLevel(windowLevel);

    QCOMPARE(image.getNumberOfWindowLevels(), 1);
    QCOMPARE(image.getWindowLevel(0), windowLevel);
}

void test_Image::addWindowLevel_ShouldNotAddWindowLevel_data()
{
    QTest::addColumn<WindowLevel>("windowLevel");

    QTest::newRow("window zero") << WindowLevel(0.0, 200.0);
}

void test_Image::addWindowLevel_ShouldNotAddWindowLevel()
{
    QFETCH(WindowLevel, windowLevel);

    Image image;
    image.addWindowLevel(windowLevel);

    QCOMPARE(image.getNumberOfWindowLevels(), 0);
}

void test_Image::addWindowLevel_ShouldAddSameWindowLevelTwoTimes()
{
    WindowLevel windowLevel(100.0, 300.0);
    Image image;
    image.addWindowLevel(windowLevel);
    image.addWindowLevel(windowLevel);

    QCOMPARE(image.getNumberOfWindowLevels(), 2);
    QCOMPARE(image.getWindowLevel(0), windowLevel);
    QCOMPARE(image.getWindowLevel(1), windowLevel);
}

void test_Image::setWindowLevelList_SetsValuesAsExpected_data()
{
    QTest::addColumn<QList<WindowLevel> >("inputList");
    QTest::addColumn<QList<WindowLevel> >("expectedSetList");

    QTest::newRow("Input list is empty") << QList<WindowLevel>() << QList<WindowLevel>();

    WindowLevel validWL(1.0, 2.0, "VALID WINDOW");
    WindowLevel invalidWL(0.0, 2.0, "INVALID WINDOW");
    
    QList<WindowLevel> validItemsList;
    validItemsList << validWL << validWL;
    QTest::newRow("Input list has only valid items") << validItemsList << validItemsList;

    validItemsList.clear();
    validItemsList << validWL << invalidWL << validWL;

    QList<WindowLevel> expectedResultList;
    expectedResultList << validWL << validWL;
    QTest::newRow("Input list has invalid items") << validItemsList << expectedResultList;
}

void test_Image::setWindowLevelList_SetsValuesAsExpected()
{
    QFETCH(QList<WindowLevel>, inputList);
    QFETCH(QList<WindowLevel>, expectedSetList);

    Image image;
    image.setWindowLevelList(inputList);

    QCOMPARE(image.getNumberOfWindowLevels(), expectedSetList.count());
    
    for (int i = 0; i < image.getNumberOfWindowLevels(); ++i)
    {
        QCOMPARE(image.getWindowLevel(i), expectedSetList.at(i));
    }
}

void test_Image::getWindowLevel_ShouldReturnExpectedValues_data()
{
    QTest::addColumn<QList<WindowLevel> >("windowLevelList");
    QTest::addColumn<int>("index");
    QTest::addColumn<WindowLevel>("expectedWindowLevel");

    
    WindowLevel wl1(10.0, 20.0, "WINDOW 1");
    WindowLevel wl2(11.0, 21.0, "WINDOW 2");
    WindowLevel wl3(12.0, 22.0, "WINDOW 3");

    QList<WindowLevel> windowLevelsList;
    windowLevelsList << wl1 << wl2 << wl3;
    
    QTest::newRow("index out of range: below 0") << windowLevelsList << -1 << WindowLevel();
    QTest::newRow("index out of range: greater than list size") << windowLevelsList << windowLevelsList.size() << WindowLevel();
    QTest::newRow("index in bounds [0]") << windowLevelsList << 0 << wl1;
    QTest::newRow("index in bounds [1]") << windowLevelsList << 1 << wl2;
    QTest::newRow("index in bounds [2]") << windowLevelsList << 2 << wl3;
}

void test_Image::getWindowLevel_ShouldReturnExpectedValues()
{
    QFETCH(QList<WindowLevel>, windowLevelList);
    QFETCH(int, index);
    QFETCH(WindowLevel, expectedWindowLevel);

    Image image;
    image.setWindowLevelList(windowLevelList);
    
    QCOMPARE(image.getWindowLevel(index), expectedWindowLevel);
}

void test_Image::getDisplayShutterForDisplayAsVtkImageData_ShouldReturnNull_data()
{
    QTest::addColumn<QList<DisplayShutter> >("shutters");

    QList<DisplayShutter> shuttersList;

    shuttersList << DisplayShutter();
    QTest::newRow("single undefined shape shutter") << shuttersList;

    DisplayShutter shutter1;
    shutter1.setPoints(QPoint(1, 1), 5);
    
    DisplayShutter shutter2;
    shutter2.setPoints(QPoint(10, 10), 2);

    shuttersList.clear();
    shuttersList << shutter1 << shutter2;
    QTest::newRow("two shutters with no intersection") << shuttersList;
}

void test_Image::getDisplayShutterForDisplayAsVtkImageData_ShouldReturnNull()
{
    QFETCH(QList<DisplayShutter>, shutters);

    Image *image = new Image;
    image->setDisplayShutters(shutters);

    QVERIFY(!image->getDisplayShutterForDisplayAsVtkImageData());
}

void test_Image::getDisplayShutterForDisplayAsVtkImageData_ShouldReturnNullWhenThereAreNoShutters()
{
    Image *image = new Image;
    QVERIFY(!image->getDisplayShutterForDisplayAsVtkImageData());
}

void test_Image::getDisplayShutterForDisplayAsVtkImageData_ShouldReturnExpectedValues_data()
{
    QTest::addColumn<int>("rows");
    QTest::addColumn<int>("columns");
    QTest::addColumn<double*>("imageOrigin");
    QTest::addColumn<double*>("imageSpacing");
    QTest::addColumn<DisplayShutter>("shutter");
    QTest::addColumn<int>("zSlice");

    double *origin1 = new double[3];
    origin1[0] = 1.3;
    origin1[1] = 2.1;
    origin1[2] = 5.6;

    double *spacing1 = new double[3];
    spacing1[0] = 0.7;
    spacing1[1] = 0.7;
    spacing1[2] = 10.0;

    DisplayShutter shutter1;
    shutter1.setPoints(QPoint(1,3), 2);

    QTest::newRow("image1 with circular shutter, zSlice = 3") << 5 << 6 << origin1 << spacing1 << shutter1 << 3;
}

void test_Image::getDisplayShutterForDisplayAsVtkImageData_ShouldReturnExpectedValues()
{
    QFETCH(int, rows);
    QFETCH(int, columns);
    QFETCH(double*, imageOrigin);
    QFETCH(double*, imageSpacing);
    QFETCH(DisplayShutter, shutter);
    QFETCH(int, zSlice);

    Image *image = new Image;

    image->setRows(rows);
    image->setColumns(columns);
    image->setImagePositionPatient(imageOrigin);
    image->setPixelSpacing(imageSpacing[0], imageSpacing[1]);
    image->addDisplayShutter(shutter);

    vtkImageData *shutterData = image->getDisplayShutterForDisplayAsVtkImageData(zSlice);

    int extent[6];
    shutterData->GetExtent(extent);

    QCOMPARE(extent[0], 0);
    QCOMPARE(extent[2], 0);
    QCOMPARE(extent[1], columns - 1);
    QCOMPARE(extent[3], rows - 1);
    QCOMPARE(extent[4], zSlice);
    QCOMPARE(extent[5], zSlice);

    double shutterSpacing[3];
    shutterData->GetSpacing(shutterSpacing);

    QCOMPARE(shutterSpacing[0], imageSpacing[0]);
    QCOMPARE(shutterSpacing[1], imageSpacing[1]);
    QCOMPARE(shutterSpacing[2], 1.0);

    double shutterOrigin[3];
    shutterData->GetOrigin(shutterOrigin);

    for (int i = 0; i < 3; ++i)
    {
        QCOMPARE(shutterOrigin[i], imageOrigin[i]);
    }
}

void test_Image::distance_ReturnsExpectedValues_data()
{
    QTest::addColumn<Image*>("image");
    QTest::addColumn<double>("expectedDistance");

    ImageOrientation pureAxial(QVector3D(1, 0, 0), QVector3D(0, 1, 0));
    ImageOrientation pureSagittal(QVector3D(0, 1, 0), QVector3D(0, 0, 1));
    ImageOrientation pureCoronal(QVector3D(1, 0, 0), QVector3D(0, 0, 1));
    ImageOrientation randomOrientation(QVector3D(1, 0.5, 2.1), QVector3D(0.3, 3.2, 0.5));
    
    Image *im1 = new Image;
    double ori1[3] = { 0.0, 0.0, 0.0 };
    im1->setImagePositionPatient(ori1);
    im1->setImageOrientationPatient(pureAxial);

    QTest::newRow("origin 0, pure axial plane") << im1 << 0.0;

    Image *im2 = new Image;
    double ori2[3] = { 2.1, 4.5, 22.1 };
    im2->setImagePositionPatient(ori2);
    im2->setImageOrientationPatient(pureAxial);

    QTest::newRow("random origin, pure axial plane") << im2 << 22.1;

    Image *im3 = new Image;
    im3->setImagePositionPatient(ori2);
    im3->setImageOrientationPatient(pureSagittal);
    QTest::newRow("random origin, pure sagittal plane") << im3 << 2.1;

    Image *im4 = new Image;
    im4->setImagePositionPatient(ori2);
    im4->setImageOrientationPatient(pureCoronal);
    QTest::newRow("random origin, pure coronal plane") << im4 << -4.5;

    Image *im5 = new Image;
    im5->setImagePositionPatient(ori2);
    im5->setImageOrientationPatient(randomOrientation);
    QTest::newRow("random origin, random plane") << im5 << 54.403;

    double ori3[3] = { -126.84476884827, -123.93805261701, -33.708148995414 };
    ImageOrientation axialLumbar(QVector3D(0.99858468770980, 0.05244236439466, 0.00885681994259), QVector3D(-0.0531411655247, 0.97706925868988, 0.20618361234664));

    Image *im6 = new Image;
    im6->setImagePositionPatient(ori3);
    im6->setImageOrientationPatient(axialLumbar);
    QTest::newRow("[lumbar] stack1 - axial plane") << im6 << -7.68022;

    double ori4[3] = { -126.49146979309, -129.31158469336, 84.0435373485925 };
    ImageOrientation axialLumbar2(QVector3D(1, -2.566479224E-06, 5.3246378229E-05), QVector3D(4.7077542149E-06, 0.99918949604034, -0.0402535349130));

    Image *im7 = new Image;
    im7->setImagePositionPatient(ori4);
    im7->setImageOrientationPatient(axialLumbar2);
    QTest::newRow("[lumbar] stack3 - axial plane") << im7 << 78.7769;
}

void test_Image::distance_ReturnsExpectedValues()
{
    QFETCH(Image*, image);
    QFETCH(double, expectedDistance);

    QVERIFY(FuzzyCompareTestHelper::fuzzyCompare(Image::distance(image), expectedDistance, 0.0001));
}

void test_Image::getPreferredPixelSpacing_ReturnsExpectedSpacing_data()
{
    QTest::addColumn<Image*>("image");
    QTest::addColumn<PixelSpacing2D>("expectedSpacing");

    PixelSpacing2D pixelSpacing(0.5, 0.5);
    double dummyFactor = MathTools::randomDouble(-20.5, 50.5);
    
    // Pixel Spacing + Estimated Radiographic Magnification Factor tests
    // It does not care about modality nor estimated radographic factor values
    {
        Image *image = new Image(this);
        Series *series = new Series(this);
        image->setParentSeries(series);
        image->setPixelSpacing(pixelSpacing);
        image->setEstimatedRadiographicMagnificationFactor(dummyFactor);

        QTest::newRow("PixelSpacing & EstimatedRadiographicMagnificationFactor(any) present => pixel spacing") << image << pixelSpacing;
    }
    
    double dummyX = MathTools::randomDouble(2.5, 5.5);
    double dummyY = MathTools::randomDouble(2.5, 5.5);
    PixelSpacing2D dummySpacing(dummyX, dummyY);
    // Pixel Spacing + Imager Pixel Spacing + Estimated Radiographic Magnification Factor tests
    // It does not care about modality, imager pixel spacing nor estimated radographic factor values
    {
        Image *image = new Image(this);
        Series *series = new Series(this);
        image->setParentSeries(series);
        image->setPixelSpacing(pixelSpacing);
        image->setImagerPixelSpacing(dummySpacing);
        image->setEstimatedRadiographicMagnificationFactor(dummyFactor);

        QTest::newRow("Any modality -PixelSpacing & ImagerPixelSpacing(any) & EstimatedRadiographicMagnificationFactor present(any) => pixel spacing") << image << pixelSpacing;
    }

    PixelSpacing2D imagerPixelSpacing(0.25, 0.25);
    // Imager Pixel Spacing test
    {
        Image *image = new Image(this);
        Series *series = new Series(this);
        image->setParentSeries(series);
        image->setImagerPixelSpacing(imagerPixelSpacing);

        QTest::newRow("Any modality - ImagerPixelSpacing present => imager pixel spacing") << image << imagerPixelSpacing;
    }

    // If modality is MG, default magnification factor is 1, so unmodified Imager Pixel Spacing should be returned
    {
        Image *image = new Image(this);
        Series *series = new Series(this);
        series->setModality("MG");
        image->setParentSeries(series);
        image->setImagerPixelSpacing(imagerPixelSpacing);

        QTest::newRow("MG - ImagerPixelSpacing present => imager pixel spacing") << image << imagerPixelSpacing;
    }
    
    double factor = 1.5;
    PixelSpacing2D magnifiedSpacing;
    magnifiedSpacing.setX(imagerPixelSpacing.x() / factor);
    magnifiedSpacing.setY(imagerPixelSpacing.y() / factor);
    // Imager Pixel Spacing + Estimated Radiographic Magnification Factor tests
    // Pixel spacing should be empty. It does not care about modality.
    // Estimated Radiographic Magnification Factor is only taken into account when modality is MG
    {
        Image *image = new Image(this);
        Series *series = new Series(this);
        series->setModality("MG");
        image->setParentSeries(series);
        image->setImagerPixelSpacing(imagerPixelSpacing);
        image->setEstimatedRadiographicMagnificationFactor(factor);

        QTest::newRow("MG - ImagerPixelSpacing & EstimatedRadiographicMagnificationFactor present => imager pixel spacing corrected by magnification factor") << image << magnifiedSpacing;
    }

    {
        Image *image = new Image(this);
        Series *series = new Series(this);
        image->setParentSeries(series);
        image->setImagerPixelSpacing(imagerPixelSpacing);
        image->setEstimatedRadiographicMagnificationFactor(dummyFactor);
        
        QTest::newRow("Any modality != MG - ImagerPixelSpacing & EstimatedRadiographicMagnificationFactor(any) present => imager pixel spacing, magnification is not used") << image << imagerPixelSpacing;
    }

    // No pixel spacing attributes present. Default constructed value should be returned
    {
        Image *image = new Image(this);
        Series *series = new Series(this);
        image->setParentSeries(series);
        image->setEstimatedRadiographicMagnificationFactor(dummyFactor);
        
        QTest::newRow("Any modality - no pixel spacing attributes present & EstimatedRadiographicMagnificationFactor(any) present => default constructed value") << image << PixelSpacing2D();
    }
}

void test_Image::getPreferredPixelSpacing_ReturnsExpectedSpacing()
{
    QFETCH(Image*, image);
    QFETCH(PixelSpacing2D, expectedSpacing);

    QVERIFY(image->getPreferredPixelSpacing().isEqual(expectedSpacing));
}

DECLARE_TEST(test_Image)

#include "test_image.moc"
