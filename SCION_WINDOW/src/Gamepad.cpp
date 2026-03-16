#include "Windowing/Inputs/Gamepad.h"
#include <Logger/Logger.h>

namespace Scion::Windowing::Inputs
{

Gamepad::Gamepad(GamepadUPtr pGamepad)
        : m_pController{std::move(pGamepad)}
        , m_mapButtons{
            {SCION_GP_BTN_A, Button{}},
            {SCION_GP_BTN_B, Button{}},
            {SCION_GP_BTN_X, Button{}},
            {SCION_GP_BTN_Y, Button{}},
            {SCION_GP_BTN_BACK, Button{}},
            {SCION_GP_BTN_GUIDE, Button{}},
            {SCION_GP_BTN_START, Button{}},
            {SCION_GP_BTN_LSTICK, Button{}},
            {SCION_GP_BTN_RSTICK, Button{}},
            {SCION_GP_BTN_LSHOULDER, Button{}},
            {SCION_GP_BTN_RSHOULDER, Button{}},
            {SCION_GP_BTN_DPAD_UP, Button{}},
            {SCION_GP_BTN_DPAD_DOWN, Button{}},
            {SCION_GP_BTN_DPAD_LEFT, Button{}},
            {SCION_GP_BTN_DPAD_RIGHT, Button{}},
        }
        , m_InstanceID{ 0U }
        , m_mapAxisValues{
            {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}
        }
        , m_JoystickHatValue{SCION_HAT_CENTERED}
{
	SDL_Joystick* joystick = SDL_GetGamepadJoystick( m_pController.get() );
	if ( !m_pController || !joystick )
		throw( "Controller or Joystick were nullptr!" );

	m_InstanceID = SDL_GetJoystickID( joystick );
	m_sName = std::string{ SDL_GetJoystickName( joystick ) };

	auto num_axis = SDL_GetNumJoystickAxes( joystick );
	auto num_balls = SDL_GetNumJoystickBalls( joystick );
	auto num_hats = SDL_GetNumJoystickHats( joystick );
	auto num_buttons = SDL_GetNumJoystickButtons( joystick );

	SCION_LOG( "Gamepad name: {}", m_sName );
	SCION_LOG( "Gamepad id: {}", m_InstanceID );
	SCION_LOG( "Gamepad num axes: {}", num_axis );
	SCION_LOG( "Gamepad num balls: {}", num_balls );
	SCION_LOG( "Gamepad num hats: {}", num_hats );
	SCION_LOG( "Gamepad num buttons: {}", num_buttons );
}

void Gamepad::Update()
{
	for ( auto& [ btn, button ] : m_mapButtons )
		button.Reset();
}

void Gamepad::OnBtnPressed( int btn )
{
	if ( btn == SCION_GP_BTN_UNKNOWN )
	{
		SCION_ERROR( "Gamepad button [{}] is unknown!", btn );
		return;
	}

	auto btnItr = m_mapButtons.find( btn );
	if ( btnItr == m_mapButtons.end() )
	{
		SCION_ERROR( "Gamepad button [{}] does not exist!", btn );
		return;
	}

	btnItr->second.Update( true );
}

void Gamepad::OnBtnReleased( int btn )
{
	if ( btn == SCION_GP_BTN_UNKNOWN )
	{
		SCION_ERROR( "Gamepad button [{}] is unknown!", btn );
		return;
	}

	auto btnItr = m_mapButtons.find( btn );
	if ( btnItr == m_mapButtons.end() )
	{
		SCION_ERROR( "Gamepad button [{}] does not exist!", btn );
		return;
	}

	btnItr->second.Update( false );
}

const bool Gamepad::IsBtnPressed( int btn ) const
{
	if ( btn == SCION_GP_BTN_UNKNOWN )
	{
		SCION_ERROR( "Gamepad button [{}] is unknown!", btn );
		return false;
	}

	auto btnItr = m_mapButtons.find( btn );
	if ( btnItr == m_mapButtons.end() )
	{
		SCION_ERROR( "Gamepad button [{}] does not exist!", btn );
		return false;
	}

	return btnItr->second.bIsPressed;
}

const bool Gamepad::IsBtnJustPressed( int btn ) const
{
	if ( btn == SCION_GP_BTN_UNKNOWN )
	{
		SCION_ERROR( "Gamepad button [{}] is unknown!", btn );
		return false;
	}

	auto btnItr = m_mapButtons.find( btn );
	if ( btnItr == m_mapButtons.end() )
	{
		SCION_ERROR( "Gamepad button [{}] does not exist!", btn );
		return false;
	}

	return btnItr->second.bJustPressed;
}

const bool Gamepad::IsBtnJustReleased( int btn ) const
{
	if ( btn == SCION_GP_BTN_UNKNOWN )
	{
		SCION_ERROR( "Gamepad button [{}] is unknown!", btn );
		return false;
	}

	auto btnItr = m_mapButtons.find( btn );
	if ( btnItr == m_mapButtons.end() )
	{
		SCION_ERROR( "Gamepad button [{}] does not exist!", btn );
		return false;
	}

	return btnItr->second.bJustReleased;
}

const bool Gamepad::IsGamepadPresent() const
{
	int count{ 0 };
	SDL_GetGamepads( &count );
	return m_pController != nullptr && count > 0;
}

const bool Gamepad::IsRumbleSupported() const
{
	if ( !m_pController )
		return false;

	SDL_PropertiesID propID = SDL_GetGamepadProperties( m_pController.get() );
	if ( propID == 0 )
		return false;

	return SDL_GetBooleanProperty( propID, SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false );
}

const Sint16 Gamepad::GetAxisPosition( Uint8 axis )
{
	auto axisItr = m_mapAxisValues.find( axis );
	if ( axisItr == m_mapAxisValues.end() )
	{
		SCION_ERROR( "Axis [{}] does not exist!", axis );
		return 0;
	}

	return axisItr->second;
}

void Gamepad::SetAxisPositionValue( Uint8 axis, Sint16 value )
{
	auto axisItr = m_mapAxisValues.find( axis );
	if ( axisItr == m_mapAxisValues.end() )
	{
		SCION_ERROR( "Axis [{}] does not exist!", axis );
		return;
	}

	axisItr->second = value;
}

void Gamepad::RumbleController( Uint16 lowFrequencyRumble, Uint16 highFrequencyRumble, Uint32 durationMs )
{
	if ( !SDL_RumbleGamepad( m_pController.get(), lowFrequencyRumble, highFrequencyRumble, durationMs ) )
	{
		SCION_WARN( "Rumble not supported for controller [{}] - Error: {}", m_InstanceID, SDL_GetError() );
	}
}
} // namespace Scion::Windowing::Inputs
