#include "grid_coord.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "debug_assert.h"


#define LINE_NOSLOPE             0xFFFFFFFF
#define GRID_ROW_NUM             2
#define GRID_COLUMN_NUM          2
#define ARRAY_LENGTH(a)          (sizeof(a) / sizeof(a[0]))


typedef struct {
	double m_slope;         // Ð±ÂÊ     / ÎÞ
	double m_intercept;     // y Öá½Ø¾à / x Öá½Ø¾à
} t_Line;

typedef struct {
	t_Polygon m_polygon[GRID_ROW_NUM][GRID_COLUMN_NUM];
} t_Grid;

static void GetLine(const t_Point point_1, const t_Point point_2, t_Line *line) {
	double slope;
	double intercept;
	double x_difference;
	double y_difference;

	ASSERT(NULL != line);
	x_difference = point_2.m_x - point_1.m_x;
	y_difference = point_2.m_y - point_1.m_y;
	if (0 == x_difference) {
		slope = LINE_NOSLOPE;
		intercept = point_1.m_x;
	}
	else {
		slope = y_difference / x_difference;
		intercept = point_1.m_y - slope * point_1.m_x;
	}

	line->m_slope = slope;
	line->m_intercept = intercept;
}

static double GetLengthInTwoPoint(const t_Point point_1, const t_Point point_2) {
	double point_dist_x;
	double point_dist_y;
	double point_dist_sum_xy;
	double point_dist_xy;

	point_dist_x = point_2.m_x - point_1.m_x;
	point_dist_y = point_2.m_y - point_1.m_y;
	point_dist_sum_xy = pow(point_dist_x, 2) + pow(point_dist_y, 2);
	point_dist_xy = sqrt(point_dist_sum_xy);
	return point_dist_xy;
}

static bool IsTopOfLine(const t_Line line, const t_Point point) {
	double line_y;

	if (LINE_NOSLOPE == line.m_slope) 
		return false;
	else {
		line_y = point.m_x * line.m_slope + line.m_intercept;
		if (line_y >= point.m_y)
			return false;
		else
			return true;
	}
}

static bool IsLeftOfLine(const t_Line line, const t_Point point) {
	double line_x;

	if (LINE_NOSLOPE == line.m_slope) {
		if (line.m_intercept <= point.m_x)
			return false;
		else
			return true;
	}
	else {
		line_x = (point.m_y - line.m_intercept) / line.m_slope;
		if (line_x <= point.m_x)
			return false;
		else
			return true;
	}
}

static bool IsInsidePolygon(const t_Polygon polygon, const t_Point point) {
	t_Point point_1;
	t_Point point_2;
	t_Line line[POLYGON_SIDE_NUM];
	int side_num;
	int side_index;
	bool is_top[POLYGON_SIDE_NUM];
	bool is_left[POLYGON_SIDE_NUM];

	side_num = ARRAY_LENGTH(line);
	for (side_index = 0; side_index < side_num; side_index++) {
		memcpy(&point_1, &polygon.m_point[side_index], sizeof(polygon.m_point[side_index]));
		if (side_num - 1 == side_index)
			memcpy(&point_2, &polygon.m_point[0], sizeof(polygon.m_point[0]));
		else
			memcpy(&point_2, &polygon.m_point[side_index + 1], sizeof(polygon.m_point[side_index + 1]));
		GetLine(point_1, point_2, &line[side_index]);

		is_top[side_index] = IsTopOfLine(line[side_index], point);
		is_left[side_index] = IsLeftOfLine(line[side_index], point);
	}

	if ((false == is_top[0] && true == is_left[1] && true == is_top[2] && false == is_left[3])
		|| (false == is_top[0] && false == is_left[1] && true == is_top[2] && true == is_left[3])
		|| (true == is_left[0] && true == is_top[1] && false == is_left[2] && false == is_top[3])
		|| (true == is_left[0] && false == is_top[1] && false == is_left[2] && true == is_top[3])
		|| (true == is_top[0] && false == is_left[1] && false == is_top[2] && true == is_left[3])
		|| (true == is_top[0] && true == is_left[1] && false == is_top[2] && false == is_left[3])
		|| (false == is_left[0] && false == is_top[1] && true == is_left[2] && true == is_top[3])
		|| (false == is_left[0] && true == is_top[1] && true == is_left[2] && false == is_top[3]))
		return true;
	else
		return false;
}

static void GetNextPoint(t_Point datum_point, t_Point aid_point, const double datum_distance, t_Point *next_point) {
	t_Line datum_line;
	double slope_angle;
	double datum_dist_x;
	double datum_dist_y;

	ASSERT(NULL != next_point);
	GetLine(datum_point, aid_point, &datum_line);
	if (LINE_NOSLOPE == datum_line.m_slope) {
		next_point->m_x = datum_point.m_x;
		if (aid_point.m_y > datum_point.m_y)
			next_point->m_y = datum_point.m_y + datum_distance;
		else
			next_point->m_y = datum_point.m_y - datum_distance;
	}
	else {
		slope_angle = atan(datum_line.m_slope);
		datum_dist_x = datum_distance * cos(slope_angle);
		datum_dist_y = datum_distance * sin(slope_angle);

		if (0 > datum_dist_x)
			datum_dist_x = 0 - datum_dist_x;
		if (0 > datum_dist_y)
			datum_dist_y = 0 - datum_dist_y;

		if (aid_point.m_x >= datum_point.m_x && aid_point.m_y >= datum_point.m_y) {
			next_point->m_x = datum_point.m_x + datum_dist_x;
			next_point->m_y = datum_point.m_y + datum_dist_y;
		}
		else if (aid_point.m_x >= datum_point.m_x && aid_point.m_y <= datum_point.m_y) {
			next_point->m_x = datum_point.m_x + datum_dist_x;
			next_point->m_y = datum_point.m_y - datum_dist_y;
		}
		else if (aid_point.m_x <= datum_point.m_x && aid_point.m_y >= datum_point.m_y) {
			next_point->m_x = datum_point.m_x - datum_dist_x;
			next_point->m_y = datum_point.m_y + datum_dist_y;
		}
		else {
			next_point->m_x = datum_point.m_x - datum_dist_x;
			next_point->m_y = datum_point.m_y - datum_dist_y;
		}
	}
}

static void GetNextPolygon(const int row_index, const int column_index, t_Polygon *p_datum_polygon, t_Polygon *p_next_polygon) {
	t_Polygon datum_polygon;
	t_Point next_point;
	t_Point temp_point;
	t_Point point_1;
	t_Point point_2;
	double line_length;
	double datum_distance;
	int point_index;
	int point_sum;

	ASSERT(NULL != p_datum_polygon && NULL != p_next_polygon);
	memcpy(&datum_polygon, p_datum_polygon, sizeof(t_Polygon));
	point_sum = ARRAY_LENGTH(datum_polygon.m_point);
	for (point_index = 0; point_index < point_sum; point_index++) {
		switch (point_index) {
			case 0:
				memcpy(&next_point, &datum_polygon.m_point[point_index], sizeof(t_Point));
				break;
			case 1:
				memcpy(&point_1, &datum_polygon.m_point[point_index - 1], sizeof(t_Point));
				memcpy(&point_2, &datum_polygon.m_point[point_index], sizeof(t_Point));
				line_length = GetLengthInTwoPoint(point_1, point_2);
				datum_distance = line_length / (GRID_COLUMN_NUM - column_index);
				GetNextPoint(point_1, point_2, datum_distance, &next_point);	
				break;
			case 2:
				memcpy(&point_1, &datum_polygon.m_point[point_index + 1], sizeof(t_Point));
				memcpy(&point_2, &datum_polygon.m_point[point_index], sizeof(t_Point));
				line_length = GetLengthInTwoPoint(point_1, point_2);
				datum_distance = line_length / (GRID_COLUMN_NUM - column_index);
				GetNextPoint(point_1, point_2, datum_distance, &temp_point);

				memcpy(&point_1, &p_next_polygon->m_point[point_index - 1], sizeof(t_Point));
				memcpy(&point_2, &temp_point, sizeof(t_Point));
				line_length = GetLengthInTwoPoint(point_1, point_2);
				datum_distance = line_length / (GRID_ROW_NUM - row_index);
				GetNextPoint(point_1, point_2, datum_distance, &next_point);
				break;
			case 3:
				memcpy(&point_1, &datum_polygon.m_point[point_index - 3], sizeof(t_Point));
				memcpy(&point_2, &datum_polygon.m_point[point_index], sizeof(t_Point));
				line_length = GetLengthInTwoPoint(point_1, point_2);
				datum_distance = line_length / (GRID_ROW_NUM - row_index);
				GetNextPoint(point_1, point_2, datum_distance, &next_point);
				break;
			default:
				break;
		}
		memcpy(&p_next_polygon->m_point[point_index], &next_point, sizeof(t_Point));
	}
	memcpy(&p_datum_polygon->m_point[0], &p_next_polygon->m_point[1], sizeof(t_Point));
	memcpy(&p_datum_polygon->m_point[3], &temp_point, sizeof(t_Point));
}

static void CreateGrid(const t_Polygon polygon, t_Grid *p_grid) {
	t_Polygon datum_polygon;
	t_Polygon next_polygon;
	t_Polygon first_polygon;
	t_Polygon last_polygon;
	int grid_row;
	int grid_column;
	int row_index;
	int column_index;

	ASSERT(NULL != p_grid);
	grid_row = ARRAY_LENGTH(p_grid->m_polygon);
	grid_column = ARRAY_LENGTH(p_grid->m_polygon[0]);
	memcpy(&datum_polygon, &polygon, sizeof(t_Polygon));
	for (row_index = 0; row_index < grid_row; row_index++) {
		if (0 == row_index)
			NULL;
		else {
			memcpy(&datum_polygon.m_point[0], &first_polygon.m_point[3], sizeof(t_Point));
			memcpy(&datum_polygon.m_point[1], &last_polygon.m_point[2], sizeof(t_Point));
			memcpy(&datum_polygon.m_point[3], &polygon.m_point[3], sizeof(t_Point));
		}
		for (column_index = 0; column_index < grid_column; column_index++) {
			GetNextPolygon(row_index, column_index, &datum_polygon, &next_polygon);
			memcpy(&p_grid->m_polygon[row_index][column_index], &next_polygon, sizeof(t_Polygon));
			if (0 == column_index)
				memcpy(&first_polygon, &next_polygon, sizeof(t_Polygon));

			else if (grid_column - 1 == column_index)
				memcpy(&last_polygon, &next_polygon, sizeof(t_Polygon));
			else
				continue;
		}
	}
}

static bool GetCoord(const t_Polygon polygon, const t_Point point, t_Coord *p_coord, t_Polygon *p_grid_polygon) {
	t_Grid grid;
	t_Coord coord;
	t_Polygon grid_polygon;
	int grid_row_num;
	int grid_column_num;
	int grid_row_index;
	int grid_column_index;
	bool is_inside;

	ASSERT(NULL != p_coord && NULL != p_grid_polygon);
	grid_row_num = ARRAY_LENGTH(grid.m_polygon);
	grid_column_num = ARRAY_LENGTH(grid.m_polygon[0]);
	CreateGrid(polygon, &grid);
	for (grid_row_index = 0; grid_row_index < grid_row_num; grid_row_index++) {
		for (grid_column_index = 0; grid_column_index < grid_column_num; grid_column_index++) {
			memcpy(&grid_polygon, &grid.m_polygon[grid_row_index][grid_column_index], sizeof(t_Polygon));
			is_inside = IsInsidePolygon(grid_polygon, point);
			if (true == is_inside)
				break;
			else
				continue;
		}
		if (true == is_inside)
			break;
		else
			continue;
	}
	if (grid_row_index == grid_row_num && grid_column_index == grid_column_num) 
		return false;
	else {
		coord.m_row = grid_row_index;
		coord.m_column = grid_column_index;
		memcpy(p_coord, &coord, sizeof(coord));
		memcpy(p_grid_polygon, &grid_polygon, sizeof(grid_polygon));
		return true;
	}
}

bool GetGridCoord(const t_Polygon polygon, const t_Point point, t_GridCoord *p_grid_coord) {
	t_Polygon grid_polygon;
	t_Coord coord;
	int coord_num;
	int coord_index;
	bool ok_coord;

	ASSERT(NULL != p_grid_coord);
	memcpy(&grid_polygon, &polygon, sizeof(polygon));
	coord_num = ARRAY_LENGTH(p_grid_coord->m_coord);
	for (coord_index = 0; coord_index < coord_num; coord_index++) {
		ok_coord = GetCoord(grid_polygon, point, &coord, &grid_polygon);
		if (false == ok_coord) 
			return false;
		else
			memcpy(&p_grid_coord->m_coord[coord_index], &coord, sizeof(coord));
	}
	return true;
}

int GetGridCoordSimilary(const t_GridCoord radar_grid_coord, const t_GridCoord camera_grid_coord) {
	const int c_BasicSimilary = 20;
	t_Coord radar_coord;
	t_Coord camera_coord;
	int coord_num;
	int coord_index;
	int similay;

	similay = 0;
	coord_num = sizeof(radar_grid_coord.m_coord) / sizeof(radar_grid_coord.m_coord[0]);
	for (coord_index = 0; coord_index < coord_num; coord_index++) {
		memcpy(&radar_coord, &radar_grid_coord.m_coord[coord_index], sizeof(t_Coord));
		memcpy(&camera_coord, &camera_grid_coord.m_coord[coord_index], sizeof(t_Coord));
		switch (coord_index) {
			case 0:
				if (radar_coord.m_row == camera_coord.m_row)
					similay += c_BasicSimilary;
				if (radar_coord.m_column == camera_coord.m_column)
					similay += c_BasicSimilary;
				break;
			case 1:
				if (40 != similay)
					NULL;
				else {
					if (radar_coord.m_row == camera_coord.m_row)
						similay += c_BasicSimilary;
					if (radar_coord.m_column == camera_coord.m_column)
						similay += c_BasicSimilary;
				}
				break;
			case 2:
				if (80 != similay) 
					NULL;
				else {
					if (radar_coord.m_row == camera_coord.m_row)
						similay += c_BasicSimilary;
					if (radar_coord.m_column == camera_coord.m_column)
						similay += c_BasicSimilary;
				}
				break;
			default:
				break;
		}
	}
	return similay;
}



#if 0
int main() {
/*
	t_Polygon polygon = {{{10, 50}, {30, 50}, {30, 10}, {10, 10}}};
	t_Point point = {15, 35};*/
/*
	t_Polygon polygon = {{{-30, 50}, {-10, 50}, {-10, 10}, {-30, 10}}};
	t_Point point = {-25, 35};*/
	t_Polygon polygon = {{{10, 10}, {10, 50}, {30, 50}, {30, 10}}};
	t_Point point = {15, 35};
	t_GridCoord grid_coord;
	int error_code;
	
	error_code = GetGridCoord(polygon, point, &grid_coord);

	return 0;
}
#endif
