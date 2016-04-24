#ifndef SSY_DEVICE_CONFIG_H
#define SSY_DEVICE_CONFIG_H


#ifdef __cplusplus
extern "C" {
#endif

	void GetConfigPath(char *p_config_path, const int config_path_size);
	void GetRadarArea(const char *p_config_path, void *pv_radar_area);
	void GetCameraArea(const char *p_config_path, void *pv_camera_area);
	void GetPastNumOfRadarDataList(const char *p_config_path, int *p_past_num);
	void GetMemberNumOfRadarDataList(const char *p_config_path, int *p_member_num);
	void GetBasicWidthOfRadarCompareArea(const char *p_config_path, int *p_basic_width);
	void GetBasicLengthOfRadarCompareArea(const char *p_config_path, int *p_basic_length);
	void GetTotalWidthOfRadarCompareArea(const char *p_config_path, int *p_total_width);
	void GetTotalLengthOfRadarCompareArea(const char *p_config_path, int *p_total_length);
	void GetPortNumOfSerialInterface(const char *p_config_path, int *p_port_num);
	void GetBaudRateOfSerialInterface(const char *p_config_path, int *p_baud_rate);

#ifdef __cplusplus
};
#endif


#endif // SSY_DEVICE_CONFIG_H