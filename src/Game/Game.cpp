#include "Game.h"
#include <fstream>
#include <iostream>
#include <set>
#include <SFML/Graphics.hpp>
#include "../Engine/TextureManager.h"
#include "../Engine/Globals.h"
#include "../Engine/Maths.h"
#include "../Engine/Timer.h"
#include "../Engine/Animation/AnimDef.h"
#include "../Engine/Input/Keyboard.h"

static std::shared_ptr<AnimationDictionary> PLAYER_ANIM_DICT = nullptr;
float currentFrameDuration = 0.f;
int frameIdx = 0;
std::string prevAnimName = "idle_down";

Game::Game() :
	Updateable(),
	m_player(),
	m_level("tiled_export\\starter_town.tmj"),
	m_cameraPosition(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenCentre)
{
	UIMANAGER.Load("ui.xml");

	m_player.SetLevel(&m_level);

	m_player.SetPosition(35.f * 32.f, 17.f * 32.f);
	m_cameraPosition = m_player.GetPosition();

	m_renderer.BuildBatches(m_level.GetRenderData(), m_level.GetLayers());

	PLAYER_ANIM_DICT = AnimationDictionary::Create("animation\\player_boy.xml");
}

Game::~Game() = default;

void Game::Update(const float deltaTime)
{
	m_player.Update(deltaTime);

	m_cameraPosition = maths::SmoothDamp(m_cameraPosition, m_player.GetPosition(), m_cameraVelocity, 0.25, deltaTime);

	m_renderer.SetCameraCentre(m_cameraPosition, m_level.GetNumColumns(), m_level.GetNumRows());

	currentFrameDuration += deltaTime;
	const std::string animName = Player::GetAnimationName(m_player.GetAnimationStateFromMovement());

	if (animName != prevAnimName)
	{
		const float currentClipDuration = static_cast<float>(PLAYER_ANIM_DICT->GetClip(prevAnimName).m_Frames[frameIdx].
			m_DurationMS) / 1000.f;
		if (currentFrameDuration > currentClipDuration)
		{
			std::cout << "Resetting! animName=" << animName << " prevAnimName=" << prevAnimName << "\n";
			frameIdx = 0;
			currentFrameDuration = 0;
			prevAnimName = animName;
		}
	}
	else
	{
		const float currentClipDuration = static_cast<float>(PLAYER_ANIM_DICT->GetClip(animName).m_Frames[frameIdx].
			m_DurationMS) / 1000.f;
		if (currentFrameDuration > currentClipDuration)
		{
			frameIdx = (frameIdx + 1) % PLAYER_ANIM_DICT->GetClip(animName).m_Frames.size();
			std::cout << "Next frame: " << frameIdx << "\n";
			currentFrameDuration = 0;
		}
	}
}

void Game::Render(sf::RenderWindow& window) const
{
	// TODO: Figure out how to draw the Entities at their relevant z-levels!
	m_renderer.Render(window);

#if BUILD_DEBUG
	sf::RectangleShape player({ 32, 32 });
	player.setFillColor({ 0, 0, 255, 128 });
	player.setPosition(m_player.GetPosition());
	window.draw(player);

	sf::CircleShape cameraReticle(10);
	player.setOrigin({ cameraReticle.getLocalBounds().size.x / 2.f, cameraReticle.getLocalBounds().size.y / 2.f });
	cameraReticle.setFillColor({ 255, 255, 255, 128 });
	cameraReticle.setPosition(m_cameraPosition);
	window.draw(cameraReticle);
#endif

	// TODO: Come up with a proper way to render all the Entities in the level
	// TODO: Make this generic and reusable - everything has walking animations
	// TODO: Make a generic "EntityRenderer", in the same vein as the TileRenderer
	sf::Sprite playerSprite(PLAYER_ANIM_DICT->GetSpriteSheet());
	const std::string animName = Player::GetAnimationName(m_player.GetAnimationStateFromMovement());
	const Animation& currentAnimClip = PLAYER_ANIM_DICT->GetClip(prevAnimName == "UNKNOWN" ? animName : prevAnimName);
	const AnimationFrame& currentFrame = currentAnimClip.m_Frames[frameIdx];

	playerSprite.setTextureRect({
		{ static_cast<int>(currentFrame.m_TopLeftX), static_cast<int>(currentFrame.m_TopLeftY) },
		{ static_cast<int>(currentFrame.m_SpriteWidth), static_cast<int>(currentFrame.m_SpriteHeight) }
		});

	playerSprite.setOrigin({ playerSprite.getLocalBounds().size.x / 2, playerSprite.getLocalBounds().size.y / 2 });
	playerSprite.setPosition(m_player.GetPosition() + sf::Vector2f{ 16, 0 });


	playerSprite.setScale({ 2, 2 });

	window.draw(playerSprite);


	window.setView({
		static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenCentre),
		static_cast<sf::Vector2f>(GRAPHIC_SETTINGS.GetScreenDetails().m_ScreenSize)
		});

	// Draw the background items first
	UIMANAGER.RenderBackground(window);

	// Then the midground stuff
	UIMANAGER.RenderMidground(window);

	// Then, in front of everything, the foreground stuff
	UIMANAGER.RenderForeground(window);

#if !BUILD_MASTER
	DrawText(window, sf::Vector2f{ 0, 10 }, 30, "%.1fFPS", Timer::Get().Fps());
#endif
}
