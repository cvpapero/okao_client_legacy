/*
終了時に人物の顔画像を保存

2014.11.12-----------------
やること
発見した人物をstackしていく
service形式
*/
 

#include <ros/ros.h>
#include <string>
#include <iostream>
#include "okao_client/OkaoStack.h"
#include <humans_msgs/Humans.h>
#include <sensor_msgs/Image.h>

//test
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

static const std::string OPENCV_WINDOW = "Image window";
using namespace std;

#define ALL 14

/*
class property{
public:
  string name;
  string laboratory;
  string grade;
};
*/

map<int, humans_msgs::Person> stack;
map<int, sensor_msgs::Image> imgstack;

class OkaoStackServer
{
private:

  ros::NodeHandle n;
  ros::ServiceServer add;
  ros::ServiceServer send;


public:

  OkaoStackServer()
  {
    add = n.advertiseService("stack_add", &OkaoStackServer::addData, this);
    send = n.advertiseService("stack_send", &OkaoStackServer::sendData, this);

    //ファイル読み込み
    for(int i = 0; i<ALL; ++i)
      {
	stringstream image_name;
	image_name <<"/home/yhirai/catkin_ws/src/okao_client/src/images/" << i << ".jpg";
	cv::Mat src = cv::imread(image_name.str());

	sensor_msgs::Image output;
	//cv::Mat outcutImage;
	//cv::resize(rgbImage, outcutImage, cv::Size(128,128));
	output.height = src.rows; 
	output.width = src.cols;
	output.encoding = "bgr8";//idToEncoding( outcutImage.type() );
	output.step 
	  = src.cols * src.elemSize();
	output.data.assign(src.data, src.data + size_t(src.rows*src.step));
	//stack.request.image = output;
	imgstack[ i ] = output;
      }

    //test
    //cv::namedWindow(OPENCV_WINDOW);
  }

  ~OkaoStackServer()
  {

    map<int, sensor_msgs::Image>::iterator imgit = imgstack.begin();
    int okao_id_count = 0;
    while( imgit != imgstack.end() )
      { 
	//cv::destroyWindow(OPENCV_WINDOW);
	cv_bridge::CvImagePtr cv_ptr;
	try
	  {
	    cv_ptr = cv_bridge::toCvCopy( imgit->second, sensor_msgs::image_encodings::BGR8);
	  }
	catch (cv_bridge::Exception& e)
	  {
	    ROS_ERROR("cv_bridge exception: %s", e.what());
   
	  }
	
	cv::Mat out = cv_ptr->image;
	stringstream ss;
	ss <<"/home/yhirai/catkin_ws/src/okao_client/src/images/" << okao_id_count <<".jpg";
	cv::imwrite(ss.str(),cv_ptr->image);
	 
	++imgit;
	++okao_id_count;
      }
    stack.clear();
    imgstack.clear();
  }

  bool addData(okao_client::OkaoStack::Request &req,
		okao_client::OkaoStack::Response &res)
  {

    //if(stack.count(req.okao_id) == 0)
    //  {
	//humans_msgs::Person prop;
	//prop = req.person;
	//prop.laboratory = req.laboratory;
	//prop.grade = req.grade;

    if(stack[req.person.okao_id].conf < req.person.conf)
      {
	stack[ req.person.okao_id ] = req.person;//prop;
    
	//if(stac)
	imgstack[ req.person.okao_id ] = req.image;
	cout <<"add---> okao_id: "<< req.person.okao_id 
	     <<", name: "<< req.person.name << endl;
      }
	/*
      }
    else
      {
	//cout << "already exists!" << endl;
      }
	*/
    /*
    cv_bridge::CvImagePtr cv_ptr;
    try
      {
	cv_ptr = cv_bridge::toCvCopy(req.image, sensor_msgs::image_encodings::BGR8);
      }
    catch (cv_bridge::Exception& e)
      {
	ROS_ERROR("cv_bridge exception: %s", e.what());
   
      }

    cv::Mat out = cv_ptr->image;
    stringstream ss;
    ss <<"/home/uema/catkin_ws/src/okao_client/src/images/" << req.person.okao_id <<".jpg";
    cv::imwrite(ss.str(),cv_ptr->image);
    */
    return true;
  }

  bool sendData(okao_client::OkaoStack::Request &req,
		okao_client::OkaoStack::Response &res)
  {
    if(stack.count(req.person.okao_id) != 0)
      {
	res.person = stack[ req.person.okao_id ];
	res.image = imgstack[ req.person.okao_id ];


	cout <<"req---> okao_id: "<< req.person.okao_id 
	     <<", name: "<< stack[ req.person.okao_id ].name << endl;

	//test
	//cv_bridge::CvImagePtr cv_ptr;
	//cv_ptr = cv_bridge::toCvCopy(res.image, sensor_msgs::image_encodings::BGR8);
	//cv::imshow(OPENCV_WINDOW, cv_ptr->image);
	//cv::waitKey(3);

	return true;
      }
    else
      {
	cout << "request false" << endl;
	return false;
      }
  }

};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "okao_stack");
  OkaoStackServer OSSObject;
  ros::spin();
  return 0;
}