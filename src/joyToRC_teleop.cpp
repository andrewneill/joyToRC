/*
Joystick to RC mapper

the node brings in values from the "joy" node and publishes to the apm/send_rc topic
which is the node used by the roscopter. Currently, the program is using an old-school
style controller but other controllers could easily be substuted

By Andrew Neill
Colorado School of Mines

9/12/2014

*/

#include <ros/ros.h>
#include <roscopter/RC.h>
#include <sensor_msgs/Joy.h>


class TeleopRC
{
public:
  TeleopRC();

private:
  void joyCallback(const sensor_msgs::Joy::ConstPtr& joy);
  
  ros::NodeHandle nh_;

  int roll_, pitch_, throttle_, yaw_, opt1, opt2, opt3, mode_, mode_a, mode_b;
  double r_scale_, p_scale_, t_scale_, y_scale_;
  ros::Publisher RC_pub_;
  ros::Subscriber joy_sub_;
  
};


TeleopRC::TeleopRC():
  roll_(1),
  pitch_(2),
  throttle_(3),
  yaw_(4),
  mode_(5)
  //  mode_a(5),
  //  mode_b(6)
{
  // Read in the program parameters, these are defined in the launch file
  // So, these values can be adjusted based on the joystick being used, etc.
  nh_.param("axis_roll", roll_, roll_);
  nh_.param("axis_pitch", pitch_, pitch_);
  nh_.param("axis_throttle", throttle_, throttle_);
  nh_.param("axis_yaw", yaw_, yaw_);
  nh_.param("scale_roll", r_scale_, r_scale_);
  nh_.param("scale_pitch", p_scale_, p_scale_);
  nh_.param("scale_throttle", t_scale_, t_scale_);
  nh_.param("scale_yaw", y_scale_, y_scale_);
  //nh_.param("mode_select13", mode_a, mode_a);
  //nh_.param("mode_select14", mode_b, mode_b);
  nh_.param("mode_select", mode_, mode_);

  // Create the publisher node for the send_rc topic
  RC_pub_ = nh_.advertise<roscopter::RC>("apm/send_rc",1);

  // Subscribe to the joystick's publications
  joy_sub_ = nh_.subscribe<sensor_msgs::Joy>("joy", 10, &TeleopRC::joyCallback, this);

}

void TeleopRC::joyCallback(const sensor_msgs::Joy::ConstPtr& joy)
{
  roscopter::RC rc;

  // Begin by choosing a mode. This isn't the first thing to be published, just the first computation
  //int a, b;
  int Mode = joy->buttons[mode_];
  // Read the values on the joystick "buttons" topic as defined by the mode_a and mode_b parameters
  // buttons is the buttons array with the values of the buttons, axes for the axes values
  
  if (Mode==0)
  {
    // Stabilize mode
    rc.channel[4] = (int) 1150;
  }
  else if(Mode==1)
  {
    // Altitude Hold
    rc.channel[4] = (int) 1650;
  }
  else
  {
    // Stabilize
    rc.channel[4] = (int) 1150;
  }


  /* //This is the old saitek control with the toggle on the front
  a = joy->buttons[mode_];
  b = joy->buttons[mode_];

  if (a == 0 && b == 0)
  {
    // Off is selected
    // Land
    rc.channel[4] = (int) 1850;
  } 
  else if(a == 1 && b == 0)
  {
    // A is selected
    // Stabilize
    rc.channel[4] = (int) 1150;
  }
  else if(a == 0 && b == 1)
  {
    // B is selected
    // Altitude hold
    rc.channel[4] = (int) 1650;
  }
  else
  {
    // Land
    rc.channel[4] = (int) 1850;
  }
  */

  // Then, transform the joystick axes values to the RC controls inputs
  // The joystick publishes all values from -1:1, so they need to be scaled for the RC
  // The RC standard is values from 1000-2000, but we don't need the full range,
  // so I'm using the "scale" values to provide a proportional control
  rc.channel[0] = (int) (1500+(r_scale_*joy->axes[roll_]));
  rc.channel[1] = (int) (1500+(p_scale_*joy->axes[pitch_]));
  rc.channel[2] = (int) (1300+(t_scale_*joy->axes[throttle_]));
  rc.channel[3] = (int) (1500+(y_scale_*joy->axes[yaw_]));
  rc.channel[5] = (int) 0; // Option 1
  rc.channel[6] = (int) 0; // Option 2
  rc.channel[7] = (int) 0; // Option 3

  // And finally, publish all the values to the apm/send_rc topic
  RC_pub_.publish(rc);
}


int main(int argc, char** argv)
{
  ros::init(argc, argv, "joyToRC");
  TeleopRC joyToRC;

  ros::spin();
}