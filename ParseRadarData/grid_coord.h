#ifndef SSY_GRID_COORD_H
#define SSY_GRID_COORD_H


#define POLYGON_SIDE_NUM            4   // 四边形（不能改动）

#ifndef __cplusplus
#define bool   int
#define false  0
#define true   1
#endif

typedef struct {
	double m_x; 
	double m_y;   
} t_Point;

typedef struct {
	t_Point m_point[POLYGON_SIDE_NUM];    
} t_Polygon;

typedef struct {
	int m_row;  
	int m_column;
} t_Coord;

typedef struct {
	t_Coord m_coord[3];
} t_GridCoord;

#ifdef __cplusplus
extern "C" {
#endif

	bool GetGridCoord(const t_Polygon polygon, const t_Point point, t_GridCoord *p_grid_coord);
	int  GetGridCoordSimilary(const t_GridCoord radar_grid_coord, const t_GridCoord camera_grid_coord);

#ifdef __cplusplus
};
#endif


#endif // SSY_GRID_COORD_H