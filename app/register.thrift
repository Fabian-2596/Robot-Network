service RobotController {
    void registerRobot(1: string name),
    bool checkRobotHealth(1: i32 id)
}