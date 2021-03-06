/*
2014.10.31---------------------------------
どうするか？


2014.10.30---------------------------------
対応漏れの処理
対応漏れしたデータのパブリッシュ
1.顔がとれていない場合
→正規のトピックにパブリッシュ
2.頭がとれていない場合
→特別にトピックが作るか？


2014.10.27---------------------------------
body_idにゼロが入る


2014.10.25---------------------------------
kinect v2とokao serverをバインドするモジュール

okaoがなくても人物位置をパブリッシュ可能

callbackのなかでpublishしたい
http://answers.ros.org/question/59725/publishing-to-a-topic-via-subscriber-callback-function/

*/

#include <iostream>
#include <sstream>
#include <ros/ros.h>
#include <humans_msgs/Humans.h>
#include <tf/transform_listener.h>
#include "MsgToMsg.hpp"

#define OKAO_NUM 3

using namespace std;

class SubscribeAndPublish
{

  //tf::TransformListener listener;

public:
  SubscribeAndPublish()
  {
    bind_pub_ = nh.advertise<humans_msgs::Humans>("/humans/BindData",10);
    kinect_sub_=  nh.subscribe("/humans/KinectV2", 1, &SubscribeAndPublish::callback, this);
  }

  /*
  void transform(int body_id, double *px, double *py, double *pz)
  {
    std::stringstream frame_id_stream;
    std::string frame_id;
    frame_id_stream << "user_" << body_id << "/Head";
    frame_id = frame_id_stream.str(); 
    std::cout << "frame_id: " << frame_id <<std::endl;
    tf::StampedTransform transform;
    try{
      listener.waitForTransform("map", frame_id, ros::Time(), ros::Duration(1.0));
      listener.lookupTransform("map", frame_id,
			       ros::Time(), transform);
      *px = transform.getOrigin().x();
      *py = transform.getOrigin().y();
      *pz = transform.getOrigin().z();
    }
    catch (tf::TransformException ex){
      ROS_ERROR("%s", ex.what());
      ros::Duration(1.0).sleep();
      *px = *py = *pz = 0;
    }
  }
  */

  void callback(const humans_msgs::HumansConstPtr& kinect)
  {
    //Subscribe  
    humans_msgs::HumansConstPtr okao = ros::topic::waitForMessage<humans_msgs::Humans>("/humans/OkaoServer", nh);
    //認識人数の取得
    int b_num = kinect->num;
    int f_num = okao->num;
 
    int no_face = 0;   

    humans_msgs::Humans bind;
    int test=0;
    bind.num = b_num;
    cout<< "body detect: "<<b_num <<endl;
    cout<< "face detect: "<<f_num <<endl;
    //リサイズ
    bind.human.resize(b_num);
    for(int i = 0; i < b_num; ++i)
      { 
	//一位から三位までの配列を確保
	bind.human[i].face.persons.resize(OKAO_NUM);
      }
    //std::cout<< "faces ok" << std::endl;
    for(int b_index = 0; b_index < b_num; ++b_index)
      {
	double head_x = kinect->human[b_index].body.joints[3].position_color_space.x;      
	double head_y = kinect->human[b_index].body.joints[3].position_color_space.y;      
	
	if( f_num )
	  {
	    //ROS_INFO("face exist");
	    for(int f_index = 0; f_index < f_num; ++f_index) 
	      {
		double face_lt_x = okao->human[f_index].face.position.lt.x;
		double face_lt_y = okao->human[f_index].face.position.lt.y;
		double face_rb_x = okao->human[f_index].face.position.rb.x;
		double face_rb_y = okao->human[f_index].face.position.rb.y;
		std::cout << "x: " << face_lt_x  << " < "  << head_x << " < " << face_rb_x << std::endl;
		std::cout << "y: " << face_lt_y  << " < "  << head_y << " < " << face_rb_y << std::endl;
		
		if((face_lt_x <= head_x) && (face_lt_y <= head_y) && (head_x <= face_rb_x) && (head_y <= face_rb_y))
		  {
		    std::cout << "head in face" << std::endl;
		    MsgToMsg::bodyAndFaceToMsg( kinect->human[b_index].body, okao->human[f_index].face, &bind.human[b_index] );
		  }
	      }
	  }       
	else
	  {
	    //対応漏れ
	    ++no_face;
	  }	
      }	
    /* 
    if( b_num && (no_face == b_num))
      {
	//ROS_INFO("no_face");
	for(int b_index = 0; b_index < b_num; ++b_index)
	  {
	    int body_id = kinect->human[b_index].body.body_id;
	    //ROS_INFO("body_id: %d",body_id);
	    bind_msg.human[b_index].body.body_id = body_id+1;
	    bind_msg.human[b_index].body.pose2d.x = kinect->human[b_index].body.pose2d.x;
	    bind_msg.human[b_index].body.pose2d.y = kinect->human[b_index].body.pose2d.y;
	    //tf
	    double px, py, pz;
	    transform(body_id , &px, &py, &pz);
	    bind_msg.human[b_index].body.pose3d.position.x = px;
	    bind_msg.human[b_index].body.pose3d.position.y = py;
	    bind_msg.human[b_index].body.pose3d.position.z = pz;
	  }
      }
    */
    std::cout << "-----------------------------" << std::endl;
    for (int i = 0; i < b_num ; ++i)
      {
	std::cout << "tracking_id[ " << i << " ]:  " << bind.human[i].body.tracking_id << std::endl;
      }
    bind.header.stamp = ros::Time::now();
    bind.header.frame_id = "bind";
    bind_pub_.publish(bind);    
    std::cout << "*****************************" << std::endl;
  }

private:
  ros::NodeHandle nh;
  ros::Publisher bind_pub_;
  ros::Subscriber kinect_sub_;
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "kinect_okao_bind");
  SubscribeAndPublish SAPObject;
  ros::spin();

  return 0;
}  

