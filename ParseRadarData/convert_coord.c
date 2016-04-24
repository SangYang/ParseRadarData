#include "convert_coord.h"
#include <stdio.h>
#include <math.h>
#include "debug_assert.h"


static double GetAreaDistanceX(const double area_near_x, const double area_far_x) {
	double area_dist_x;
	ASSERT(area_near_x < area_far_x);
	area_dist_x = area_far_x - area_near_x;
	return area_dist_x;
}

static double GetAreaDistanceY(const double area_near_y, const double area_far_y) {
	double area_dist_y;
	ASSERT(area_near_y < area_far_y);
	area_dist_y = area_far_y - area_near_y;
	return area_dist_y;
}

static double GetRadarAngleXY(const double radar_pos_x, const double radar_pos_y) {
	double radar_angle_xy;

	ASSERT(0 != radar_pos_y);
	radar_angle_xy = atan(radar_pos_x / radar_pos_y);
	return radar_angle_xy;
}

static double GetRadarAngleXYZ(const double radar_height, const double radar_pos_x, const double radar_pos_y) {
	double radar_angle_xyz;
	double radar_pos_dist_xy;
	double radar_pos_sum_xy;
	ASSERT(0 < radar_height);
	radar_pos_sum_xy = pow(radar_pos_x, 2) + pow(radar_pos_y, 2);
	radar_pos_dist_xy = sqrt(radar_pos_sum_xy);
	radar_angle_xyz = atan(radar_pos_dist_xy / radar_height);
	return radar_angle_xyz;
}




static double GetCameraAreaIntersectionXY(const double area_near_x, const double area_far_x, const double radar_pos_x, const double radar_pos_y) {
	double area_dist_x;
	double radar_angle_xy;
	double camera_area_interseciton_xy;

	area_dist_x = GetAreaDistanceX(area_near_x, area_far_x);
	radar_angle_xy = GetRadarAngleXY(radar_pos_x, radar_pos_y);
	ASSERT(0 < radar_angle_xy && M_PI > radar_angle_xy);
	camera_area_interseciton_xy = area_dist_x / sin(radar_angle_xy);
	return camera_area_interseciton_xy;
}









static double GetRadarAngleXZ(const double radar_height, const double radar_pos_x) {
	double radar_angle_xz;

	ASSERT(0 < radar_height);
	radar_angle_xz = atan(radar_pos_x / radar_height);   
	return radar_angle_xz;
}


static double GetCameraAngleXZ(const double camera_height, const double area_near_x, const double area_far_x) {
	double camera_angle_xz;
	double area_middle_x;

	ASSERT(0 < camera_height);
	ASSERT(area_near_x < area_far_x);
	area_middle_x = area_near_x + (area_far_x - area_near_x) / 2;
	camera_angle_xz = atan(area_middle_x / camera_height);
	return camera_angle_xz;
}

static double GetCameraAngleXY(const double area_near_x, const double area_far_x, const double area_near_y, const double area_far_y) {
	double camera_angle_xy;
	double area_middle_x;
	double area_middle_y;

	ASSERT(area_near_x < area_far_x);
	ASSERT(area_near_y < area_far_y);
	area_middle_x = area_near_x + (area_far_x - area_near_x) / 2;
	area_middle_y = area_near_y + (area_far_y - area_near_y) / 2;
	ASSERT(0 != area_middle_y);
	camera_angle_xy = atan(area_middle_x / area_middle_y);
	return camera_angle_xy;
}


static double GetCameraMidAreaDistanceXZ(const double camera_height, const double area_near_x, const double area_far_x) {
	double camera_area_dist_xz;
	double area_middle_x;
	double camera_area_sum_xz;
	
	ASSERT(area_near_x < area_far_x);
	area_middle_x = area_near_x + (area_far_x - area_near_x) / 2;
	camera_area_sum_xz = pow(camera_height, 2) + pow(area_middle_x, 2);
	camera_area_dist_xz = sqrt(camera_area_sum_xz);
	return camera_area_dist_xz;
}

static double GetAreaProjectionDistanceXZ(const double camera_height, const double area_near_x, const double area_far_x) {
	double area_dist_x;
	double camera_angle_xz;
	double area_project_xz;

	ASSERT(area_near_x < area_far_x);
	area_dist_x = area_far_x - area_near_x;
	camera_angle_xz = GetCameraAngleXZ(camera_height, area_near_x, area_far_x);
	area_project_xz = area_dist_x * cos(camera_angle_xz);
	return area_project_xz;
}

double GetRadarPosInAreaProjectionDistanceXZ(const double radar_height, const double radar_pos_x, const double camera_height, const double area_near_x, const double area_far_x) {
	double area_project_xz;
	double area_noproject_xz;
	double radar_angle_xz;
	double camera_angle_xz;
	double angle_difference_xz;
	double camera_area_dist_xz;
	double pos_in_halfproject_dist_xz;
	double pos_in_project_dist_xz;

	radar_angle_xz = GetRadarAngleXZ(radar_height, radar_pos_x);
	camera_angle_xz = GetCameraAngleXZ(camera_height, area_near_x, area_far_x);
	angle_difference_xz = camera_angle_xz - radar_angle_xz;
	area_project_xz = GetAreaProjectionDistanceXZ(camera_height, area_near_x, area_far_x);
	area_noproject_xz = area_project_xz * tan(camera_angle_xz);
	camera_area_dist_xz = GetCameraMidAreaDistanceXZ(camera_height, area_near_x, area_far_x);
	pos_in_halfproject_dist_xz = (camera_area_dist_xz - area_noproject_xz / 2) * tan(angle_difference_xz);
	pos_in_project_dist_xz = area_project_xz / 2 - pos_in_halfproject_dist_xz;
	return pos_in_project_dist_xz;
}


/*

static double GetAreaAngleXY(const double area_near_x, const double area_far_x, const double area_near_y, const double area_far_y) {
	double area_dist_x;
	double area_dist_y;
	double area_angle_xy;

	ASSERT(area_near_x < area_far_x);
	ASSERT(area_near_y < area_far_y);
	area_dist_x = area_far_x - area_near_x;
	area_dist_y = area_far_y - area_near_y;
	ASSERT(0 != area_dist_y);
	area_angle_xy = atan(area_dist_x / area_dist_y);
	return area_angle_xy;
}

static double GetCameraMidAreaDistanceXY(const double area_near_x, const double area_far_x, const double area_near_y, const double area_far_y) {
	double camera_area_dist_xy;
	double camera_area_sum_xy;
	double area_middle_x;
	double area_middle_y;

	ASSERT(area_near_x < area_far_x);
	ASSERT(area_near_y < area_far_y);
	area_middle_x = area_near_x + (area_far_x - area_near_x) / 2;
	area_middle_y = area_near_y + (area_far_y - area_near_y) / 2;
	camera_area_sum_xy = pow(area_middle_x, 2) + pow(area_middle_y, 2);
	camera_area_dist_xy = sqrt(camera_area_sum_xy);
	return camera_area_dist_xy;
}

static double GetAreaProjectionDistanceXY(const double area_near_x, const double area_far_x, const double area_near_y, const double area_far_y) {
	double area_dist_x;
	double area_dist_y;
	double camera_angle_xy;
	double area_angle_xy;
	double area_diagonal_xy;
	double area_project_xy;

	ASSERT(area_near_x < area_far_x);
	ASSERT(area_near_y < area_far_y);
	area_dist_x = area_far_x - area_near_x;
	area_dist_y = area_far_y - area_near_y;
	camera_angle_xy = GetCameraAngleXZ(area_near_x, area_far_x, area_near_y, area_far_y);
	area_angle_xy = GetAreaAngleXY(area_near_x, area_far_x, area_near_y, area_far_y);
	area_diagonal_xy = area_dist_x / sin(area_angle_xy);
	area_project_xy = area_diagonal_xy * cos(camera_angle_xy + area_angle_xy - M_PI / 2);
	return area_project_xy;
}


double GetRadarPosInAreaProjectionDistanceXY(const double radar_height, const double radar_pos_x, const double camera_height, const double area_near_x, const double area_far_x) {
	double area_project_xz;
	double area_noproject_xz;
	double radar_angle_xz;
	double camera_angle_xz;
	double angle_difference_xz;
	double camera_area_dist_xz;
	double pos_in_halfproject_dist_xz;
	double pos_in_project_dist_xz;

	radar_angle_xz = GetRadarAngleXZ(radar_height, radar_pos_x);
	camera_angle_xz = GetCameraAngleXZ(camera_height, area_near_x, area_far_x);
	angle_difference_xz = camera_angle_xz - radar_angle_xz;
	area_project_xz = GetAreaProjectionDistanceXZ(camera_height, area_near_x, area_far_x);
	area_noproject_xz = area_project_xz * tan(camera_angle_xz);
	camera_area_dist_xz = GetCameraMidAreaDistanceXZ(camera_height, area_near_x, area_far_x);
	pos_in_halfproject_dist_xz = (camera_area_dist_xz - area_noproject_xz / 2) * tan(angle_difference_xz);
	pos_in_project_dist_xz = area_project_xz / 2 - pos_in_halfproject_dist_xz;
	return pos_in_project_dist_xz;     
}*/



#if 0
int main() {
	const double c_RadarHeight = 5;
	const double c_CameraHeight = 5;
	const double c_AreaNear = 30;
	const double c_AreaFar = 90;
	const double c_RadarPosX = 40;
	double pos_in_project_dist_xz;

	pos_in_project_dist_xz = GetRadarPosInAreaProjectionDistanceXZ(c_RadarHeight, c_RadarPosX, c_CameraHeight, c_AreaNear, c_AreaFar);

	return 0;
}
#endif