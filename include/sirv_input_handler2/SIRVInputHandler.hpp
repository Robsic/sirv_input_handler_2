#include "joystick.hpp"

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <iostream>

using namespace std::chrono_literals;

class SIRVInputPublisher : public rclcpp::Node
{
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;

public:
  SIRVInputPublisher() : Node("sirv_input_handler")
  {
    publisher_ = this->create_publisher<std_msgs::msg::String>("sirv_joystick", 10);
  }

  void publish(std::string msg)
  {
    auto message = std_msgs::msg::String();
    message.data = msg;
    RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", message.data.c_str());
    publisher_->publish(message);
  }
};

class SIRVInputHandler
{
  std::shared_ptr<SIRVInputPublisher> node;
  std::vector<Joystick *> js;
  JoystickEvent event;
  rclcpp::TimerBase::SharedPtr timer_;

  void timer_callback()
  {
    std::stringstream sstm;
    for (auto joystick : js)
    {
      if (joystick->sample(&event))
      {
        if (event.isButton())
        {

          sstm << "Button " << (uint)event.number << " value " << event.value;
          node->publish(sstm.str());
        }
        else if (event.isAxis())
        {
          sstm << "Axis " << (uint)event.number << " is at position " << event.value;
          node->publish(sstm.str());
        }
      }
    }
  }

public:
  SIRVInputHandler(int argc, const char *argv[])
  {
    if (argc > 1)
    {
      for (int i = 1; i < argc; i++)
      {
        std::stringstream sstm;
        sstm << "/dev/input/" << argv[i];
        Joystick *a = new Joystick(sstm.str());
        if (a->isFound())
          js.push_back(a);
      }
      std::cout << "Found " << js.size() << " joysticks.\n";
    }

    rclcpp::init(argc, argv);
    node = std::make_shared<SIRVInputPublisher>();
    timer_ = node->create_wall_timer(100ms, std::bind(&SIRVInputHandler::timer_callback, this));
    rclcpp::spin(node);
  };
  ~SIRVInputHandler() { rclcpp::shutdown(); };
};