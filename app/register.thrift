service RobotController {
    void ping(),
    void registerRobot(1: i32 id, 2: string name),
    bool checkRobotHealth(1: i32 id),
    void electionResult(1: i32 id)
}