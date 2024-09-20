#ifndef CAMERA_H
#define CAMERA_H

class CCamera
{
public:
    void Init();
    void Refresh();
    void SetPos(float x, float y, float z);
    void GetPos(float &x, float &y, float &z);
    void SetYaw(float angle);
    float GetYaw();
    void SetPitch(float angle);
    float GetPitch();

    void Move(float speed, bool positive);
    void Strafe(float speed, bool positive);
    void RotateYaw(float angle);
    void RotatePitch(float angle);
private:
    float m_x, m_y, m_z;
    float m_lx, m_ly, m_lz;
    float m_yaw, m_pitch;
    float m_strafe_lx, m_strafe_lz;
};

#endif