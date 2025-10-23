#ifndef SEPARATING_AXIS_THEOREM_H
#define SEPARATING_AXIS_THEOREM_H

#include <unordered_set>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include "Globals.h"

class CollisionPolygon : public sf::Drawable, public sf::Transformable
{
public:
	// Constructor
	CollisionPolygon(float radius, uint8_t numVertices, sf::Color color = { 255, 255, 255 });

	// TODO: Support arbitrary placement of vertices
	void Reset(float radius, uint8_t numVertices);

	float GetRadius() const { return m_vertices[0].position.length(); }
	size_t GetNumVertices() const { return m_vertices.getVertexCount() - 1; }

	sf::Vector2f GetVelocity() const { return m_velocity; }
	void SetVelocity(const sf::Vector2f& velocity) { m_velocity = velocity; }

	float GetAngularVelocity() const { return m_angularVelocity; }
	void SetAngularVelocity(const float angularVelocity) { m_angularVelocity = angularVelocity; }

	void SetColor(const sf::Color& color);

	// Returns true if the point is within the polygon
	bool Contains(const sf::Vector2f& point);

	// Returns the vertex and normal data of the polygon
	void GetVertexData(std::unordered_set<sf::Vector2f, Vector2HashFunction>& normals, sf::VertexArray& vertices) const;

	static bool DetectCollision(const CollisionPolygon& p1, const CollisionPolygon& p2, float& minPenetration, sf::Vector2f& penetrationAxis);

#if BUILD_DEBUG
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
#endif

private:
	sf::VertexArray m_vertices;
	sf::Vector2f m_velocity;
	float m_angularVelocity;
	sf::Color m_color;

	static float GetVertexRotationAmountRadians(uint8_t numVertices);
};


#endif
