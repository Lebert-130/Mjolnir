#ifndef CAMERA_H
#define CAMERA_H

class CCamera
{
public:
    void Init();
    void Refresh();
    void SetPos(Vector pos);
    void GetPos(Vector& pos);
    void SetYaw(float angle);
    float GetYaw();
    void SetPitch(float angle);
    float GetPitch();

    void Move(float speed, bool positive);
    void Strafe(float speed, bool positive);
    void RotateYaw(float angle);
    void RotatePitch(float angle);
private:
    Vector m_vecPosition;
    Vector m_veclPosition;
    float m_yaw, m_pitch;
    float m_strafe_lx, m_strafe_lz;
};

#endif