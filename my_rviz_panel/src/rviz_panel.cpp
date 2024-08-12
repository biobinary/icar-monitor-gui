// Copyright (c) 2011, Willow Garage, Inc.
// All rights reserved.
//
// Software License Agreement (BSD License 2.0)
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Willow Garage, Inc. nor the names of its
//       contributors may be used to endorse or promote products derived from
//       this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "rviz_panel.hpp"

#include <QColor>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>
#include <QLCDNumber>
#include <QTimer>
#include <QTime>

#include <memory>

#include "rviz_common/display.hpp"
#include "rviz_common/render_panel.hpp"
#include "rviz_common/ros_integration/ros_node_abstraction.hpp"
#include "rviz_common/visualization_manager.hpp"
#include "rviz_common/window_manager_interface.hpp"
#include "rviz_rendering/render_window.hpp"
#include "rviz_common/display_group.hpp"

// BEGIN_TUTORIAL
// Constructor for MyViz.  This does most of the work of the class.
MyViz::MyViz(
  QApplication * app,
  rviz_common::ros_integration::RosNodeAbstractionIface::WeakPtr rviz_ros_node,
  QWidget * parent)
: QMainWindow(parent), app_(app), rviz_ros_node_(rviz_ros_node)
{

  // Add visualization.
  QWidget * central_widget = new QWidget();
  QHBoxLayout * main_layout = new QHBoxLayout(central_widget);

  central_widget->setStyleSheet("background-color: #201E43;");
  main_layout->setSpacing(0);
  main_layout->setMargin(0);

  // Set Up The RVIZ Render Panel
  
  app_->processEvents();
  render_panel_ = std::make_shared<rviz_common::RenderPanel>(central_widget);
  app_->processEvents();
  render_panel_->getRenderWindow()->initialize();
  auto clock = rviz_ros_node_.lock()->get_raw_node()->get_clock();
  rviz_common::WindowManagerInterface * wm = nullptr;
    
  manager_ = std::make_shared<rviz_common::VisualizationManager>(
  render_panel_.get(), rviz_ros_node_, wm, clock);
  
  render_panel_->initialize(manager_.get());
  app_->processEvents();
  manager_->initialize();
  manager_->startUpdate();

  // ---------------------------------------------------------------------------------------

  // Set the Background Color of the RVIZ's Render Panel 
  global_options_ = manager_->getRootDisplayGroup()->subProp("Global Options");
  global_options_->subProp("Background Color")->setValue("#201E43");

  QWidget *left_widget = new QWidget();
  left_widget->setStyleSheet("background-color: #201E43;");

  QVBoxLayout *left_layout = new QVBoxLayout(left_widget);

  QLabel *clock_label = new QLabel("00:00:00");
  clock_label->setAlignment(Qt::AlignCenter);
  clock_label->setStyleSheet("font-size: 20px; color: #EEEEEE;");
  left_layout->addWidget(clock_label);

  QWidget *speed_widget = new QWidget();
  QVBoxLayout *speed_layout = new QVBoxLayout(speed_widget);

  QLabel *speed_counter = new QLabel("0");
  speed_counter->setAlignment(Qt::AlignCenter);
  speed_counter->setStyleSheet("font-size: 52px; color: #EEEEEE;");

  QLabel *speed_label = new QLabel("KM/H");
  speed_label->setAlignment(Qt::AlignCenter);
  speed_label->setStyleSheet("font-size: 26px; color: #EEEEEE;");

  speed_layout->addStretch(1);
  speed_layout->addWidget(speed_counter);
  speed_layout->addWidget(speed_label);
  speed_layout->addStretch(1);

  left_layout->addStretch(1);
  left_layout->addWidget(speed_widget);
  left_layout->addStretch(1);

  main_layout->addWidget(left_widget, 1);
  main_layout->addWidget(render_panel_.get(), 2);

  central_widget->setLayout(main_layout);
  setCentralWidget(central_widget);
  resize(1280, 720);

  grid_ = manager_->createDisplay("rviz_default_plugins/Grid", "adjustable grid", true);
  if (grid_ == NULL)
    throw std::runtime_error("Error creating grid display");

  grid_->subProp("Color")->setValue("#EEEEEE");
  grid_->subProp("Plane Cell Count")->setValue(25);

  robot_model_ = manager_->createDisplay("rviz_default_plugins/RobotModel", "Robot Model", true);
  if(robot_model_ == NULL)
    throw std::runtime_error("Error creating Robot Model display");

  robot_model_->subProp("Description Topic")->setValue("robot_description");

  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, [clock_label]() {
    clock_label->setText(QTime::currentTime().toString("hh:mm:ss"));
  });
  timer->start(1000); 

}

void MyViz::closeEvent(QCloseEvent * event)
{
  QMainWindow::closeEvent(event);
  rclcpp::shutdown();
}