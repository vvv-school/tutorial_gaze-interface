// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
//
// Author: Ugo Pattacini - <ugo.pattacini@iit.it>

#include <functional>
#include <cmath>

#include <gazebo/common/Plugin.hh>
#include <gazebo/physics/World.hh>
#include <gazebo/physics/Model.hh>
#include <gazebo/common/Events.hh>
#include <ignition/math/Pose3.hh>

namespace gazebo {

/******************************************************************************/
class Mover : public gazebo::WorldPlugin
{
    gazebo::physics::WorldPtr world;
    gazebo::physics::ModelPtr ball;
    gazebo::event::ConnectionPtr renderer_connection;
    ignition::math::Pose3d init_pose;
    double t0;

    /**************************************************************************/
    void onWorld() {
        const auto t = world->SimTime().Double() - t0;
        const auto phi = 2. * M_PI * (t / 10.);
        
        const auto x = .4 * std::cos(phi);
        const auto y = .2 * std::sin(phi);
        const auto z = y;

        const auto& p = init_pose.Pos();
        const auto& q = init_pose.Rot();
        ignition::math::Pose3d pose(p.X() + x, p.Y() + y, p.Z() + z,
                                    q.W(), q.X(), q.Y(), q.Z());
        ball->SetWorldPose(pose);
    }

public:
    /**************************************************************************/
    void Load(gazebo::physics::WorldPtr world, sdf::ElementPtr) override {
        this->world = world;
        ball = world->ModelByName("tutorial_gaze-interface-ball");

        auto bind = std::bind(&Mover::onWorld, this);
        renderer_connection = gazebo::event::Events::ConnectWorldUpdateBegin(bind);

        init_pose = ball->WorldPose();
        t0 = world->SimTime().Double();
    }
};

}

GZ_REGISTER_WORLD_PLUGIN(gazebo::Mover)
