#pragma once
#include "Structs.h"

class PtzCmd
{
public:
	typedef std::shared_ptr<PtzCmd> Ptr;


    /**
     * 云台指令码计算
     *
     * @param leftRight  镜头左移右移 0:停止 1:左移 2:右移
     * @param upDown     镜头上移下移 0:停止 1:上移 2:下移
     * @param inOut      镜头放大缩小 0:停止 1:缩小 2:放大
     * @param moveSpeed  镜头移动速度 默认 0XFF (0-255)
     * @param zoomSpeed  镜头缩放速度 默认 0X1 (0-255)
     */
    static std::string cmdString(int leftRight, int upDown, int inOut, int moveSpeed, int zoomSpeed);

    /**
     * @brief 云台指令码计算
     *
     * @param fourthByte 第四个字节
     * @param fifthByte  第五个字节
     * @param sixthByte  第六个字节
     * @param seventhByte  第七个字节
     * @return std::string
     */
    static std::string cmdCode(int fourthByte, int fifthByte, int sixthByte, int seventhByte);


};



class PtzParser
{
public:
    PtzParser() = default;
    ~PtzParser() = default;

    int ParseControlCmd(control_cmd_t& ctrlcmd, const std::string& cmdstr);

private:
    void parse_ptz(const char* b, control_cmd_t& ctrlcmd);
    void parse_fi(const char* b, control_cmd_t& ctrlcmd);
    void parse_preset(const char* b, control_cmd_t& ctrlcmd);
    void parse_patrol(const char* b, control_cmd_t& ctrlcmd);
    void parse_scan(const char* b, control_cmd_t& ctrlcmd);

private:
    uint8_t m_b4;
    uint8_t m_b5;
    uint8_t m_b6;
    uint8_t m_b7;
};

