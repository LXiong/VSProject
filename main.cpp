/*
    Author:                 Bryan Herd
    Last Modified Date:     2013/08/30
*/
/*
    Main function for the Video Surveillance Project
    TODO: Modify existing functions into .cpp and .h class files for
    cleaner code.
    TODO: Merge motion detection code into facial recognition code
    TODO: Separate and add license plate detection/recognition code
*/

//OpenCV Headers
#include "opencv2/core/core.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

//Created Class Headers
#include "camera.h"
#include "User.h"

//Standard Headers
#include <iostream>
#include <fstream>
#include <sstream>

//Standard and OpenCV namespaces
using namespace cv;
using namespace std;

//Maximum number of cameras per user
const int MAX_NUM_CAMERAS = 24;

//Function to read CSV file containing images to train on
//Source at: http://docs.opencv.org/trunk/modules/contrib/doc/facerec/tutorial/facerec_video_recognition.html
static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';')
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

//Helper function to check if file exists
    bool file_exists(const char *filename)
    {
        ifstream file(filename);
        return file.good();
    }

int main(int argc, char* argv[])
{
    //Check if OpenCV Path was provided
    /*if(argv[1] == NULL)
    {
        cerr << "Program usage: ./program /path/to/opencv/installation/\n";
        exit(EXIT_SUCCESS);
    }*/
    //Set OpenCV Path
    //Temporarily filled for testing
    //string opencv_dir = argv[1];
    string opencv_dir = "/home/bryan/SummerProject2013/opencv-2.4.6.1/";

    // Get the path to the CSV and HaarCascade/LBPCascade files
    // Files are for front and profile of faces
    string fn_haar_front;
    fn_haar_front += opencv_dir+string("data/haarcascades/haarcascade_frontalface_alt2.xml"); /* "home/bryan/SummerProject2013/opencv-2.4.6.1/data/haarcascade_frontalface_alt2.xml" for fast Haar training*/
    string fn_haar_profile;
    fn_haar_profile += opencv_dir+string("data/haarcascades/haarcascade_profileface.xml");
    string fn_lbp_front;
    fn_lbp_front += opencv_dir + string("data/lbpcascades/lbpcascade_frontalface.xml");
    string fn_lbp_profile;
    fn_lbp_profile += opencv_dir+string("data/lbpcascades/lbpcascade_profileface.xml");
    string fn_csv = "/home/bryan/SummerProject2013/VideoSurveillanceProject/data/data.csv";

    //Strings for username and password to IP Camera
    vector<camera> usercam(24);

    usercam[0].setUser("test");
    usercam[0].setPwd("test01pass33");
    //string usr = "test";
    //string pwd = "test01pass33";

    //Temporary use of video to track faces
    //TODO: Change to camera stream when available
    //const string& fn_video = "/home/bryan/SummerProject2013/VideoSurveillanceProject/sample_data/out.avi";

    //Foscam URL manipulation: http://www.zoneminder.com/wiki/index.php/Foscam
    //TODO: Create class to handle various types of IP Cameras
    string fn_video;
    try
    {
        fn_video += string("http://192.168.1.135:8091/videostream.asf?user=")+usercam.at(0).getUser()+string("&pwd=")+usercam.at(0).getPwd()+string("&resolution=8&rate=6"); //Resolution is 320x240 at 15fps
    }
    catch(Exception e)
    {
        fn_video = "/home/bryan/SummerProject2013/VideoSurveillanceProject/sample_data/out.avi";  //Replace with exception handling
    }

    // These vectors hold the images and corresponding labels:
    vector<Mat> images;
    vector<int> labels;

    // Read in the data (fails if no valid input filename is given, but you'll get an error message):
    try
    {
        read_csv(fn_csv, images, labels);
    }
    catch (cv::Exception& e)
    {
        cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
        exit(1);
    }

    // Get the height from the first image to reshape later images
    // Used for Eigenfaces and Fisherfaces
    int im_width = images[0].cols;
    int im_height = images[0].rows;

    //
    bool trainModel = 1;
    // Create a FaceRecognizer and try to load the training data, or train and save to file:
    Ptr<FaceRecognizer> model = createLBPHFaceRecognizer();
    if(file_exists("LBPHTrainedModel.xml") && trainModel)
    {
        model->load("LBPHTrainedModel.xml");
    }
    else
    {
        model->train(images, labels);
        model->save("LBPHTrainedModel.xml");
    }

    //Declare and load haar cascade classifier
    //Change either to lbp or haar cascade
    //Haar cascade and lbp can be used together, but will slow down processing alot
    //If GPU is used (OpenCL), then speed can be improved 5x
    CascadeClassifier lbp_cascade_facefrontal;
    CascadeClassifier lbp_cascade_faceprofile;
    CascadeClassifier haar_cascade_facefrontal;
    CascadeClassifier haar_cascade_faceprofile;

    try
    {
        haar_cascade_facefrontal.load(fn_haar_front);
        haar_cascade_faceprofile.load(fn_haar_profile);
        lbp_cascade_facefrontal.load(fn_lbp_front);
        lbp_cascade_faceprofile.load(fn_lbp_profile);
    }
    catch(Exception e)
    {

    }

    if(lbp_cascade_facefrontal.empty() && lbp_cascade_faceprofile.empty())
    {
            cerr << "ERROR: Could not load cascade (" << fn_haar_front << ")" << fn_lbp_profile << "!\n";
            exit(EXIT_FAILURE);

    }

    // Get a handle to the Video device:
    VideoCapture cap(fn_video);

    // Check if we can use this device at all:
    if(!cap.isOpened()){cout << "Video is not open at source: " << fn_video <<"!\n";return -1;}

    // Holds the current frame from the Video device:
    Mat frame;
    for(;;) {
        cap >> frame;
        // Clone the current frame:
        Mat original = frame.clone();

        // Convert the current frame to grayscale:
        Mat gray;
        if(original.channels() == 3)
        {
            cvtColor(original, gray, CV_BGR2GRAY);
        }
        else if(original.channels() == 4)
        {
            cvtColor(original, gray, CV_BGRA2GRAY);
        }
        else
        {
            gray = original;
        }

        //Resize the image to work faster
        const int DETECTION_WIDTH = 320;

        Mat smallImg;
        float scale = gray.cols/(float) DETECTION_WIDTH;

        if(gray.cols > DETECTION_WIDTH)
        {
            //Shrink and keep aspect ratio
            int scaledHeight = cvRound(original.rows/scale);
            resize(gray, smallImg, Size(DETECTION_WIDTH, scaledHeight));
        }
        else
        {
            smallImg = gray;
        }

        Mat equalizedImg;
        equalizeHist(smallImg, equalizedImg);

        //Flip the image for full profile matching
        Mat flippedImg;
        cv::flip(equalizedImg, flippedImg, 1);

        // Find the faces in the frame:
        vector< Rect_<int> > faces;


        //Flags for detectMultiScale(IMAGE_TO_DETECT, FACE_STORAGE, SCALE_FACTOR, MIN_NEIGHBORS, FLAGS, MINSIZE, MAXSIZE)
        haar_cascade_facefrontal.detectMultiScale(equalizedImg, faces, 1.3, 2, CV_HAAR_DO_CANNY_PRUNING);
        haar_cascade_faceprofile.detectMultiScale(equalizedImg, faces, 1.3, 2, CV_HAAR_DO_CANNY_PRUNING);
        haar_cascade_faceprofile.detectMultiScale(equalizedImg, faces, 1.3, 2, CV_HAAR_DO_CANNY_PRUNING);

        // At this point you have the position of the faces in
        // the vector faces.

        //Detect all the faces in the image
        for(uint i = 0; i < faces.size(); i++)
        {

            // Process face by face:
            Rect face_i = faces[i];

            // Crop the face from the image.
            Mat face = gray(face_i);

            // Now perform the prediction
            int prediction = -1;
            double confidence = 0.0;
            model->predict(face, prediction, confidence);

            //Draw a green rectangle around the detected face:
            rectangle(original, face_i, CV_RGB(0, 255,0), 1);

            // Annotate the box with text:
            string box_text = format("Prediction = %d Confidence = %d.", prediction, confidence);

            // Calculate the position for annotated text
            int pos_x = std::max(face_i.tl().x - 10, 0);
            int pos_y = std::max(face_i.tl().y - 10, 0);

            //Display on original source
            putText(original, box_text, Point(pos_x, pos_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
        }

        //Display the result
        imshow("Video_Surveillance_Test", original);

        char key = (char) waitKey(20);

        //Exit program on escape
        if(key == 27)
        {
            destroyWindow("Video_Surveillance_Test");
            cap.release();
            break;
        }

    }

    destroyAllWindows();

    return 0;
}
