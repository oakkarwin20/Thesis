#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"

Vec2 AnalogJoystick::GetPosition() const
{
	return Vec2(m_correctedPosition);
}

float AnalogJoystick::GetMagnitude() const
{
	float magnitude = m_correctedPosition.GetLength();
	return magnitude;
}

float AnalogJoystick::GetOrientationDegrees() const
{
	float orientationDegrees = Atan2Degrees( m_correctedPosition.y, m_correctedPosition.x );
	return orientationDegrees;
}

Vec2 AnalogJoystick::GetRawUncorrectedPosition() const
{
	return m_rawPosition;
}

float AnalogJoystick::GetInnerDeadZoneFraction() const
{
	return m_innerDeadZoneFraction;
}

float AnalogJoystick::GetOuterDeadZoneFraction() const
{
	return m_outerDeadZoneFraction;
}

void AnalogJoystick::Reset()
{
	m_rawPosition			= Vec2( 0.0f, 0.0f );
	m_correctedPosition		= Vec2( 0.0f, 0.0f );
}

void AnalogJoystick::SetDeadZoneThresholds(float normalizedInnerDeadZoneThreshold, float normalizedOuterDeadZoneThreshold)
{
	m_innerDeadZoneFraction = normalizedInnerDeadZoneThreshold;
	m_outerDeadZoneFraction = normalizedOuterDeadZoneThreshold;
}

void AnalogJoystick::UpdatePosition(float rawNormalizedX, float rawNormalizedY)
{
	m_rawPosition.x = rawNormalizedX;
	m_rawPosition.y = rawNormalizedY;

	float rawMagnitude = m_rawPosition.GetLength();
	float rawOrientationDegrees = m_rawPosition.GetOrientationDegrees();
	float correctedDisplacementMagnitude = RangeMapClamped( rawMagnitude, m_innerDeadZoneFraction, m_outerDeadZoneFraction, 0.0f, 1.0f );

	m_correctedPosition.SetPolarDegrees( rawOrientationDegrees, correctedDisplacementMagnitude );
}
