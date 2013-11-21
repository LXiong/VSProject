// VideoSurveillanceProject.cpp : Defines the entry point for the console application.
//

/*
    Author:                 Bryan Herd
    Last Modified Date:     2013/11/17
*/
/*
    Main function for the Video Surveillance Project
*/



//Created Class Headers
#include "camera.h"
#include "User.h"
#include "stdafx.h"

//Standard and OpenCV namespace
using namespace cv;
using namespace std;

//Error codes
#define CASCADE_ERROR -3
#define IPCAMERA_ERROR -4

//Debugging Loop number
#define LOOP_NUM 10

//Colors for drawing
const static Scalar colors[] = {  CV_RGB(0,0,255),
                                  CV_RGB(0,128,255),
                                  CV_RGB(0,255,255),
                                  CV_RGB(0,255,0),
                                  CV_RGB(255,128,0),
                                  CV_RGB(255,255,0),
                                  CV_RGB(255,0,0),
                                  CV_RGB(255,0,255)
                                } ;

//DEBUGGING METHODS AND VARIABLES START
int64 work_begin = 0;
int64 work_end = 0;
string outputName;

static void workBegin()
{
	work_begin = getTickCount();
}

static void workEnd()
{
	work_end += (getTickCount()-work_begin);
}

static double getTime()
{
	return work_end/((double) cvGetTickFrequency()*1000.);
}
//DEBUGGING METHODS AND VARIABLES END

//Maximum number of user cameras
const int MAX_NUM_CAMERAS = 24;

//Function to read CSV file containing images to train on
//Source at: http://docs.opencv.org/trunk/modules/contrib/doc/facerec/tutorial/facerec_video_recognition.html
static void readCSV(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';');

//Facial detection using GPU
static void detect( Mat& img, vector<Rect>& faces,
             ocl::OclCascadeClassifier& cascade,
             double scale, bool calTime);

//Facial detection using CPU
static void detectCPU( Mat& img, vector<Rect>& faces,
                CascadeClassifier& cascade,
                double scale, bool calTime);

//Draws around face
static void Draw(Mat& img, vector<Rect>& faces, double scale);

//Helper function to check if file exists
bool file_exists(const char *filename)
{
    ifstream file(filename);
    return file.good();
}

int main(int argc, char* argv[])
{   
    //Set OpenCV Path
	string opencv_dir = getenv("OPENCV_DIR");
	
    // Get the path to the CSV and HaarCascade/LBPCascade files
    // Files are for front and profile of faces
    string fn_haar_front;
    fn_haar_front += opencv_dir+string("/data/haarcascades/haarcascade_frontalface_default.xml"); 
    string fn_haar_profile;
    fn_haar_profile += opencv_dir+string("/data/haarcascades/haarcascade_profileface.xml");
    string fn_lbp_front;
    fn_lbp_front += opencv_dir + string("/data/lbpcascades/lbpcascade_frontalface.xml");
    string fn_lbp_profile;
    fn_lbp_profile += opencv_dir+string("/data/lbpcascades/lbpcascade_profileface.xml");

	//Path to Local Binary Pattern Histograms Trained Model
	const char* fn_TrainedModel = "LBPHTrainedModel.xml";

	//Path to CSV File created by create_csv.py to identify subjects !! Change to data.csv for desktop
    string fn_csv = "data/data.csv";

	//Use CPU to process image?
	bool useCPU = false;
	
    //Strings for username and password to IP Camera
    vector<camera> usercam(24);

	// These vectors hold the images and corresponding labels:
    vector<Mat> images;
    vector<int> labels;

	//OCL Device
	vector<ocl::Info> oclinfo;
	int devnums = ocl::getDevice(oclinfo);
	if(devnums < 1)
	{
		cout << "No device found!\n";
		return -1;
	}
	cout<< devnums;
	ocl::setDevice(oclinfo[0]);
	ocl::setBinpath("./");
	
	//Scale to use in multiscale detection
	double scale = 1.1;

	//Temporary IP Cam Test setup
    usercam[0].setUser("test");
    usercam[0].setPwd("test01pass33");

    //Foscam URL manipulation: http://www.zoneminder.com/wiki/index.php/Foscam
    //TODO: Create class to handle various types of IP Cameras
    string fn_video;
    try
    {	
			fn_video += string("http://192.168.1.135:8091/videostream.asf?user=")+usercam.at(0).getUser()+string("&pwd=")+usercam.at(0).getPwd()+string("&resolution=8&rate=6"); //Resolution is 320x240 at 15fps
	}
    catch(exception e)
    {
       cerr << "IP Camera stream not found at " << fn_video << " !\n";
	   exit(IPCAMERA_ERROR);
    }

    // Read in the data (fails if no valid input filename is given, but you'll get an error message):
    try
    {
        readCSV(fn_csv, images, labels);
    }
    catch (exception e)
    {
        cerr << "Error opening file " << fn_csv << ";" << endl;
		exit(1);
    }

	bool trainmodel = false;
    
    // Create a FaceRecognizer and try to load the training data, or train and save to file:
    Ptr<FaceRecognizer> model = createLBPHFaceRecognizer();

    if(file_exists(fn_TrainedModel) && !trainmodel)
    {
        model->load(fn_TrainedModel);
    }
    else
    {
        model->train(images, labels);
        model->save(fn_TrainedModel);
    }

    //Declare and load haar cascade classifier
    CascadeClassifier cpu_cascade_facefrontal;
    CascadeClassifier cpu_cascade_faceprofile;
    ocl::OclCascadeClassifier cascade_facefrontal;
    ocl::OclCascadeClassifier cascade_faceprofile;

    if(!cascade_facefrontal.load(fn_haar_front) || !cpu_cascade_facefrontal.load(fn_lbp_front))   
    {
		cerr << "Error: Could not load cascades!\n";
		exit(CASCADE_ERROR);
    }
	
	//Comment out cap(fn_video) when using laptop
    // Get a handle to the Video device:
    //VideoCapture cap(fn_video);
	VideoCapture cap("data/sample.avi");
	//VideoCapture cap(0);

    // Check if we can use this device at all:
    if(!cap.isOpened()){cout << "Video is not open at source: " << fn_video <<"!\n";}
	
	//Window for display
	namedWindow("Video_Surveillance", 1);

    // Holds the current frame from the Video device:
    Mat frame;
    for(;;) 
	{
        cap >> frame;

		if(frame.empty())
			break;

        // Clone the current frame:
        Mat& original = frame.clone();
		
		//Vector to hold the faces
        vector<Rect> faces;

		if(useCPU)
			detectCPU(original, faces, cpu_cascade_facefrontal, scale, false);
		else
			detect(original, faces, cascade_facefrontal, scale, false);

		Draw(original, faces, scale);

		char key = (char) waitKey(20);

        //Exit program on escape
        if(key == 27)
        {
			break;        
        }
    }

		
	    destroyWindow("Video_Surveillance");
		return 0;
}

static void readCSV(const string& filename, vector<Mat>& images, vector<int>& labels, char separator)
{
    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file)
    {
        string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
    }
    string line, path, classlabel;
    while (getline(file, line))
    {
        stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty())
        {
            images.push_back(imread(path, 0));
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
}

static void detect( Mat& img, vector<Rect>& faces, ocl::OclCascadeClassifier& cascade, double scale, bool calTime)
{
	ocl::oclMat image(img);
	ocl::oclMat gray, smallImg(cvRound(img.rows/scale), cvRound(img.cols/scale), CV_8UC1);

	//Convert image to gray
	ocl::cvtColor(image, gray, COLOR_BGR2GRAY);

	//Resize image and equalize it
	ocl::resize(gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR);
	ocl::equalizeHist(smallImg, smallImg);

	cascade.detectMultiScale(smallImg, faces, 1.1, 3, 0 | CASCADE_SCALE_IMAGE, Size(30,30), Size(0,0));
}

static void detectCPU( Mat& img, vector<Rect>& faces, CascadeClassifier& cascade, double scale, bool calTime)
{
	Mat cpu_gray, cpu_smallImg(cvRound(img.rows/scale), cvRound(img.cols/scale), CV_8UC1);
	cvtColor(img, cpu_gray, COLOR_BGR2GRAY);
	resize(cpu_gray, cpu_smallImg, cpu_smallImg.size(), 0, 0, INTER_LINEAR);
	equalizeHist(cpu_smallImg, cpu_smallImg);

	cascade.detectMultiScale(cpu_smallImg, faces, 1.1, 3, 0 | CASCADE_SCALE_IMAGE, Size(30,30), Size(0,0));
}

void Draw(Mat& img, vector<Rect>& faces, double scale)
{
	int i = 0;
    for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ )
    {
        Point center;
        Scalar color = colors[i%8];
        int radius;
        center.x = cvRound((r->x + r->width*0.5)*scale);
        center.y = cvRound((r->y + r->height*0.5)*scale);
        radius = cvRound((r->width + r->height)*0.25*scale);
        circle( img, center, radius, color, 3, 8, 0 );
		imwrite( to_string(i) + string(".jpg"), img );
    }
    
    if(abs(scale-1.0)>.001)
    {
        resize(img, img, Size((int)(img.cols/scale), (int)(img.rows/scale)));
    }
    imshow( "Video_Surveillance", img );
}

