#include <idynutils/tests_utils.h>
#include <gtest/gtest.h>
#include <trajectory_utils/trajectory_utils.h>
#include <idynutils/cartesian_utils.h>
#include <fstream>

#define dt 0.001

namespace {

class test_trajectory_generator: public trajectory_utils::trajectory_generator
{
public:
    test_trajectory_generator():
    trajectory_generator(dt)
    {}

    boost::shared_ptr<KDL::Path> _createArcPath(const KDL::Frame& start_pose,
                                                const KDL::Rotation& final_rotation,
                                                const double angle_of_rotation,
                                                const KDL::Vector& circle_center,
                                                const KDL::Vector& plane_normal){
        return createArcPath(start_pose, final_rotation, angle_of_rotation,
                             circle_center, plane_normal);
    }
};

class testArcTrj: public ::testing::Test {
public:
    testArcTrj()
    {
        start = start.Identity();
        end_rot = start.M;

        angle = M_PI_2;

        circle_center[0] = 0.0;
        circle_center[1] = 1.0;
        circle_center[2] = 0.0;

        plane_normal[0] = 1.0;
        plane_normal[1] = 0.0;
        plane_normal[2] = 0.0;
    }

    virtual ~testArcTrj() {

    }

    virtual void SetUp() {

    }

    virtual void TearDown() {

    }

    void logFrame(const KDL::Frame& F) {
        _stored_frames.push_back(F);
    }

    void writeMatlabFile(double k = -1) {
        std::string file_name = "stored_trj.m";
        file1.open(file_name.c_str());
        file1<<"trj = {"<<std::endl;

        for(unsigned int i = 0; i < _stored_frames.size(); ++i){
            KDL::Frame T = _stored_frames[i];
            file1<<"["<<T(0,0)<<" "<<T(0,1)<<" "<<T(0,2)<<" "<<T(0,3)<<"; "<<
                   T(1,0)<<" "<<T(1,1)<<" "<<T(1,2)<<" "<<T(1,3)<<"; "<<
                   T(2,0)<<" "<<T(2,1)<<" "<<T(2,2)<<" "<<T(2,3)<<"; "<<
                   T(3,0)<<" "<<T(3,1)<<" "<<T(3,2)<<" "<<T(3,3)<<"],"<<std::endl;
        }
        file1<<"};"<<std::endl;
        file1.close();

        file_name = "plot_trj.m";
        file2.open(file_name.c_str());

        file2<<"clear all; clc"<<std::endl;
        file2<<"run('startup_rvc.m') %Needs Matlab Robotics Toolbox to run!"<<std::endl;
        file2<<"run('stored_trj.m')"<<std::endl;

        file2<<"X = []; Y = []; Z = [];"<<std::endl;
        file2<<"for i = 1:1:length(trj)"<<std::endl;
        file2<<"    X(i) = trj{i}(1,4);"<<std::endl;
        file2<<"    Y(i) = trj{i}(2,4);"<<std::endl;
        file2<<"    Z(i) = trj{i}(3,4);"<<std::endl;
        file2<<"end"<<std::endl;

        file2<<"plot3(X,Y,Z,'.'); axis([-2,2,-2,2,-2,2]); hold on"<<std::endl;
        file2<<"trplot(trj{1},'axis',[-2,2,-2,2,-2,2],'color',[0 0 0]); hold on;"<<std::endl;
        file2<<"trplot(trj{length(trj)},'axis',[-2,2,-2,2,-2,2],'color',[0 0 0]); hold on;"<<std::endl;

        file2<<"for i = 2:"<<k<<":length(trj)-1"<<std::endl;
        file2<<"    trplot(trj{i},'axis',[-2,2,-2,2,-2,2]); hold on;"<<std::endl;
        file2<<"end"<<std::endl;
        file2.close();
    }



    std::ofstream file1;
    std::ofstream file2;

    std::vector<KDL::Frame> _stored_frames;

    test_trajectory_generator trj;
    KDL::Frame start;
    KDL::Rotation end_rot;

    double angle;

    KDL::Vector circle_center;

    KDL::Vector plane_normal;

};

TEST_F(testArcTrj, testCreateArcPath)
{
    boost::shared_ptr<KDL::Path> arc_path = this->trj._createArcPath(this->start, this->end_rot,
                                                                     this->angle, this->circle_center,
                                                                     this->plane_normal);
    EXPECT_FALSE(arc_path==NULL);

    std::cout<<"Path lenght: "<<arc_path->PathLength()<<std::endl;

    KDL::Frame final_pose = arc_path->Pos(arc_path->PathLength());
    std::cout<<"Final Pose:"<<std::endl;cartesian_utils::printKDLFrame(final_pose);

    KDL::Frame expected_final_pose; expected_final_pose.Identity();
    expected_final_pose.p.x(0.);
    expected_final_pose.p.y(1.);
    expected_final_pose.p.z(-1.);

    tests_utils::KDLFramesAreEqual(final_pose,expected_final_pose);

    for(unsigned int i = 0; i < int(arc_path->PathLength()*100); ++i)
        this->logFrame(arc_path->Pos(i/100.));
    this->writeMatlabFile(20);
}

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
