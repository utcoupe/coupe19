#!/usr/bin/env python
# -*-coding:Utf-8 -*

import rospy
from geometry_msgs.msg import Pose2D
from pathfinder.srv import *

class PathfinderClient(object):
    def __init__ (self):
        self.PATHFINDER_FINDPATH_SERVICE_NAME = "navigation/pathfinder/find_path"
        self.pathfinderFindPathService = ""
        self._ConnectToServer ()
    
    def _ConnectToServer (self):
        rospy.loginfo('Waiting for "' + self.PATHFINDER_FINDPATH_SERVICE_NAME + '"...')
        rospy.wait_for_service (self.PATHFINDER_FINDPATH_SERVICE_NAME)
        rospy.loginfo('Pathfinder found.')
        try:
            self.pathfinderFindPathService = rospy.ServiceProxy(self.PATHFINDER_FINDPATH_SERVICE_NAME, FindPath)
        except rospy.ServiceException, e:
            error_str = "Error when trying to connect to "
            error_str += self.PATHFINDER_FINDPATH_SERVICE_NAME
            error_str += " : " + str(e)
            rospy.logfatal (error_str)
    
    def FindPath (self, startPos, endPos, ignoreTags):
        response = ""
        try:
            response = self.pathfinderFindPathService(startPos, endPos, ignoreTags)
        except rospy.ServiceException, e:
            error_str = "Error when trying to use "
            error_str += self.PATHFINDER_FINDPATH_SERVICE_NAME
            error_str += " : " + str(e)
            rospy.logerr (error_str)
            raise Exception
        else:
            if response.return_code == response.NO_PATH_FOUND:
                raise Exception("No path found.")
            else:
                return (response.path, (response.return_code == response.START_END_POS_NOT_VALID))
