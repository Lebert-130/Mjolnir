#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>

#include "vector.h"
#include "camera.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void CCamera::Init()
{
	m_yaw = 0.0f;
	m_pitch = 0.0f;
}

void CCamera::Refresh()
{
	m_veclPosition = Vector(cos(m_yaw) * cos(m_pitch), sin(m_pitch), sin(m_yaw) * cos(m_pitch));

	m_strafe_lx = cos(m_yaw - M_PI/2);
	m_strafe_lz = sin(m_yaw - M_PI/2);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(m_vecPosition[0], m_vecPosition[1], m_vecPosition[2], 
		m_vecPosition[0] + m_veclPosition[0], m_vecPosition[1] + m_veclPosition[1], m_vecPosition[2] + m_veclPosition[2],
		0.0, 1.0, 0.0);
}

void CCamera::SetPos(Vector pos)
{
	m_vecPosition = pos;
	Refresh();
}

void CCamera::GetPos(Vector& pos)
{
	pos = m_vecPosition;
}

void CCamera::Move(float speed, bool positive)
{
	if(!positive)
		speed = -speed;

	float lx = cos(m_yaw) * cos(m_pitch);
	float ly = sin(m_pitch);
	float lz = sin(m_yaw) * cos(m_pitch);

	Vector direction(lx, ly, lz);
	direction = direction * speed;

	m_vecPosition = m_vecPosition + direction;
}

void CCamera::Strafe(float speed, bool positive)
{
	if(!positive)
		speed = -speed;

	m_vecPosition = Vector(m_vecPosition[0] + speed * m_strafe_lx, m_vecPosition[1], m_vecPosition[2] + speed * m_strafe_lz);
}

void CCamera::RotateYaw(float angle)
{
	m_yaw += angle;
}

void CCamera::RotatePitch(float angle)
{
	const float limit = 89.0 * M_PI / 180.0;

	m_pitch -= angle;

    if(m_pitch < -limit)
        m_pitch = -limit;

    if(m_pitch > limit)
        m_pitch = limit;
}

void CCamera::SetYaw(float angle)
{
    m_yaw = angle;
    Refresh();
}

float CCamera::GetYaw(){
    return m_yaw;
}

void CCamera::SetPitch(float angle)
{
    m_pitch = angle;
    Refresh();
}

float CCamera::GetPitch(){
    return m_pitch;
}