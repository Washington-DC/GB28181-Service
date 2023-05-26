#include "pch.h"
#include "PtzCmd.h"
#include <iomanip>

std::string PtzCmd::cmdString(int leftRight, int upDown, int inOut, int moveSpeed, int zoomSpeed)
{
	int cmdCode = 0;
	if (leftRight == 2) {
		cmdCode = 0x01;  // 右移
	}
	else if (leftRight == 1) {
		cmdCode = 0x02;  // 左移
	}
	if (upDown == 2) {
		cmdCode |= 0x04;  // 下移
	}
	else if (upDown == 1) {
		cmdCode |= 0x08;  // 上移
	}
	if (inOut == 2) {
		cmdCode |= 0x10;  // 放大
	}
	else if (inOut == 1) {
		cmdCode |= 0x20;  // 缩小
	}

	LOG(INFO) << "BYTE: " << fmt::format("{:08b}", cmdCode);

	std::stringstream ss;
	// 前三字节
	ss << "A50F01";
	// 字节4 指令码
	ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << cmdCode;
	// 字节5 水平控制速度
	ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << moveSpeed;
	// 字节6 垂直控制速度
	ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << moveSpeed;
	// 字节7 高四位 焦距控制速度
	ss << std::setfill('0') << std::setw(1) << std::hex << std::uppercase << zoomSpeed;
	// 字节7 低四位
	ss << "0";
	// 字节8 校验码  字节8=(字节1+字节2+字节3+字节4+字节5+字节6+字节7)%256
	int checkCode =
		(0xA5 + 0x0F + 0x01 + cmdCode + moveSpeed + moveSpeed + (zoomSpeed << 4)) % 0x100;
	ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << checkCode;

	LOG(INFO) << "Code: " << ss.str();
	return ss.str();
}

std::string PtzCmd::cmdCode(int fourthByte, int fifthByte, int sixthByte, int seventhByte)
{
	std::stringstream ss;
	// 前三字节
	ss << "A50F01";
	// 字节4 指令码
	ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << fourthByte;
	// 字节5 水平控制速度
	ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << fifthByte;
	// 字节6 垂直控制速度
	ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << sixthByte;
	// 字节7 高四位 焦距控制速度
	ss << std::setfill('0') << std::setw(1) << std::hex << std::uppercase << seventhByte;
	// 字节7 低四位
	ss << "0";
	// 字节8 校验码  字节8=(字节1+字节2+字节3+字节4+字节5+字节6+字节7)%256
	int checkCode =
		(0xA5 + 0x0F + 0x01 + fourthByte + fifthByte + sixthByte + (seventhByte & 0xF0)) % 0x100;
	ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << checkCode;

	return ss.str();
}

std::string PtzCmd::cmdLens(int iris, int focus, int iris_speed, int focus_speed)
{
	int cmdCode = 0;
	if (iris == 1)
		cmdCode = 0b01001000;  //缩小
	else if(iris == 2)
		cmdCode = 0b01000100;  //放大

	if (focus == 1)
		cmdCode |= 0b00000010;   //近
	else if(focus == 2)
		cmdCode |= 0b00000001;   //远


	LOG(INFO) << "BYTE: " << fmt::format("{:08b}", cmdCode);

	std::stringstream ss;
	// 前三字节
	ss << "A50F01";
	// 字节4 指令码
	ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << cmdCode;
	// 字节5 水平控制速度
	ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << iris_speed;
	// 字节6 垂直控制速度
	ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << focus_speed;
	// 字节7 
	ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << 0;
	
	// 字节8 校验码  字节8=(字节1+字节2+字节3+字节4+字节5+字节6+字节7)%256
	int checkCode =
		(0xA5 + 0x0F + 0x01 + cmdCode + iris_speed + focus_speed + 0x00) % 0x100;
	ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << checkCode;

	LOG(INFO) << "Code: " << ss.str();
	return ss.str();
}







int PtzParser::ParseControlCmd(control_cmd_t& ctrlcmd, const std::string& cmdstr)
{
	///parse control type
	uint8_t b[8];
	sscanf(cmdstr.c_str(), "%02x%02x%02x%02x%02x%02x%02x%02x", &b[0], &b[1], &b[2], &b[3]
		, &b[4], &b[5], &b[6], &b[7]);
	m_b4 = b[3];
	m_b5 = b[4];
	m_b6 = b[5];
	m_b7 = b[6];

	if ((m_b4 >> 6) == 0x0)
	{
		parse_ptz((const char*)b, ctrlcmd);
	}
	else if ((m_b4 >> 4) == 0x04)
	{
		parse_fi((const char*)b, ctrlcmd);
	}
	else if (m_b4 == 0x81 || m_b4 == 0x82 || m_b4 == 0x83)
	{
		parse_preset((const char*)b, ctrlcmd);
	}
	else if (m_b4 >= 0x84 && m_b4 <= 0x88)
	{
		parse_patrol((const char*)b, ctrlcmd);
	}
	else if (m_b4 == 0x89 || m_b4 == 0x8a)
	{
		parse_scan((const char*)b, ctrlcmd);
	}
	else if (m_b4 == 0x00)
	{
		ctrlcmd.ctrltype = CONTROL_STOP;
	}

	return 0;
}

void PtzParser::parse_ptz(const char* b, control_cmd_t& ctrlcmd)
{
	ctrlcmd.ctrltype = PTZ_TYPE;

	///< Zoom
	if (((m_b4 & 0x30) >> 4) == 0x01)
		ctrlcmd.ptz_zoom.cmdtype = ptz_cmd_zoom_t::ZOOM_IN;
	else if (((m_b4 & 0x30) >> 4) == 0x02)
		ctrlcmd.ptz_zoom.cmdtype = ptz_cmd_zoom_t::ZOOM_OUT;
	ctrlcmd.ptz_zoom.speed = (m_b7 & 0xf0) >> 4;

	///< Tilt
	if (((m_b4 & 0x0c) >> 2) == 0x01)
		ctrlcmd.ptz_tilt.cmdtype = ptz_cmd_tilt_t::TILT_DOWN;
	else if (((m_b4 & 0x0c) >> 2) == 0x02)
		ctrlcmd.ptz_tilt.cmdtype = ptz_cmd_tilt_t::TILT_UP;
	ctrlcmd.ptz_tilt.speed = m_b6;

	///< Pan
	if ((m_b4 & 0x03) == 0x01)
		ctrlcmd.ptz_pan.cmdtype = ptz_cmd_pan_t::PAN_RIGHT;
	else if ((m_b4 & 0x03) == 0x02)
		ctrlcmd.ptz_pan.cmdtype = ptz_cmd_pan_t::PAN_LEFT;
	ctrlcmd.ptz_pan.speed = m_b5;
}

void PtzParser::parse_fi(const char* b, control_cmd_t& ctrlcmd)
{
	ctrlcmd.ctrltype = FI_TYPE;

	///< Iris
	if (((m_b4 & 0x0c) >> 2) == 0x01)
		ctrlcmd.fi_iris.cmdtype = fi_cmd_iris_t::IFIS_AMPLIFICATION;
	else if (((m_b4 & 0x0c) >> 2) == 0x02)
		ctrlcmd.fi_iris.cmdtype = fi_cmd_iris_t::IFIS_SHRINK;
	ctrlcmd.fi_iris.speed = m_b6;

	///< Foucs
	if ((m_b4 & 0x03) == 0x01)
		ctrlcmd.fi_focus.cmdtype = fi_cmd_focus_t::FOCUS_FAR;
	else if ((m_b4 & 0x03) == 0x02)
		ctrlcmd.fi_focus.cmdtype = fi_cmd_focus_t::FOCUS_NEAR;
	ctrlcmd.fi_focus.speed = m_b5;
}

void PtzParser::parse_preset(const char* b, control_cmd_t& ctrlcmd)
{
	ctrlcmd.ctrltype = PRESET_TYPE;
	if (m_b4 == 0x81)
		ctrlcmd.preset.cmdtype = preset_cmd_t::PRESET_SET;
	else if (m_b4 == 0x82)
		ctrlcmd.preset.cmdtype = preset_cmd_t::PRESET_CALL;
	else if (m_b4 == 0x83)
		ctrlcmd.preset.cmdtype = preset_cmd_t::PRESET_DELE;
	ctrlcmd.preset.index = m_b6;
}

void PtzParser::parse_patrol(const char* b, control_cmd_t& ctrlcmd)
{
	ctrlcmd.ctrltype = PATROL_TYPE;
	ctrlcmd.patrol.patrol_id = m_b5;
	ctrlcmd.patrol.preset_id = m_b6;
	uint32_t v = m_b7 & 0xf0;
	v = (v << 4) | m_b6;
	ctrlcmd.patrol.value = v;

	if (m_b4 == 0x84)
		ctrlcmd.patrol.cmdtype = patrol_cmd_t::PATROL_ADD;
	else if (m_b4 == 0x85)
		ctrlcmd.patrol.cmdtype = patrol_cmd_t::PATROL_DELE;
	else if (m_b4 == 0x86)
		ctrlcmd.patrol.cmdtype = patrol_cmd_t::PATROL_SET_SPEED;
	else if (m_b4 == 0x87)
		ctrlcmd.patrol.cmdtype = patrol_cmd_t::PATROL_SET_TIME;
	else if (m_b4 == 0x88)
		ctrlcmd.patrol.cmdtype = patrol_cmd_t::PATROL_START;
}

void PtzParser::parse_scan(const char* b, control_cmd_t& ctrlcmd)
{
	ctrlcmd.ctrltype = SCAN_TYPE;
	uint32_t v = m_b7;
	v = (v << 4) | m_b6;
	ctrlcmd.autoscan.speed = v;
	ctrlcmd.autoscan.scan_id = m_b5;
	if (m_b4 == 0x89 && m_b6 == 0x00)
		ctrlcmd.autoscan.cmdtype = scan_cmd_t::SCAN_START;
	else if (m_b4 == 0x89 && m_b6 == 0x01)
		ctrlcmd.autoscan.cmdtype = scan_cmd_t::SCAN_SET_LEFT_BOADER;
	else if (m_b4 == 0x89 && m_b6 == 0x02)
		ctrlcmd.autoscan.cmdtype = scan_cmd_t::SCAN_SET_RIGHT_BOADER;
	else if (m_b4 == 0x8a)
		ctrlcmd.autoscan.cmdtype = scan_cmd_t::SCAN_SET_SPEED;
}


