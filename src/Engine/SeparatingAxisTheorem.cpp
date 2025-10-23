#include "SeparatingAxisTheorem.h"
#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>


CollisionPolygon::CollisionPolygon(const float radius, const uint8_t numVertices, const sf::Color color) :
	m_velocity{},
	m_angularVelocity{},
	m_color{ color }
{
	for (float theta = 0.0f; theta < 2.0f * PI - 0.01f; theta += GetVertexRotationAmountRadians(numVertices))
	{
		const sf::Vertex v{ radius * sf::Vector2f{ std::cosf(theta), -std::sinf(theta) }, m_color };
		m_vertices.append(v);
	}
	m_vertices.append(m_vertices[0]);
}

float CollisionPolygon::GetVertexRotationAmountRadians(const uint8_t numVertices)
{
	return 2.0f * PI / static_cast<float>(numVertices);
}

void CollisionPolygon::Reset(const float radius, const uint8_t numVertices)
{
	m_vertices.clear();
	m_vertices.resize(0);
	for (float theta = 0.0f; theta < 2.0f * PI - 0.01f; theta += GetVertexRotationAmountRadians(numVertices))
	{
		m_vertices.append({ radius * sf::Vector2f{ std::cosf(theta), -std::sinf(theta) }, m_color });
	}
	m_vertices.append(m_vertices[0]);
}

void CollisionPolygon::SetColor(const sf::Color& color)
{
	m_color = color;
	for (size_t i = 0; i < m_vertices.getVertexCount(); ++i)
	{
		m_vertices[i].color = color;
	}
}

bool CollisionPolygon::Contains(const sf::Vector2f& point)
{
	bool pointInsidePolygon = false;
	for (size_t i = 0; i < GetNumVertices(); ++i)
	{
		sf::Vector2f vc = getTransform().transformPoint(m_vertices[i].position);
		sf::Vector2f vn = getTransform().transformPoint(m_vertices[i + 1].position);

		if (((vc.y >= point.y && vn.y < point.y) || (vc.y < point.y && vn.y >= point.y)) &&
			(point.x < (vn.x - vc.x) * (point.y - vc.y) / (vn.y - vc.y) + vc.x))
		{
			pointInsidePolygon = !pointInsidePolygon;
		}
	}

	return pointInsidePolygon;
}

void CollisionPolygon::GetVertexData(std::unordered_set<sf::Vector2f, Vector2HashFunction>& normals, sf::VertexArray& vertices) const
{
	for (size_t i = 0; i < m_vertices.getVertexCount() - 1; ++i)
	{
		const sf::Vertex v1{ getTransform().transformPoint(m_vertices[i].position), sf::Color::Green };
		const sf::Vertex v2{ getTransform().transformPoint(m_vertices[i + 1].position), sf::Color::Green };

		vertices.append(v1);
		normals.insert((v2.position - v1.position).perpendicular().normalized());
	}
}

bool CollisionPolygon::DetectCollision(const CollisionPolygon& p1, const CollisionPolygon& p2, float& minPenetration, sf::Vector2f& penetrationAxis)
{
	// Gets the vertices and normals of the polygons
	std::unordered_set<sf::Vector2f, Vector2HashFunction> normals;
	sf::VertexArray leftVertices;
	sf::VertexArray rightVertices;

	p1.GetVertexData(normals, leftVertices);
	p2.GetVertexData(normals, rightVertices);

	// We need the minimum penetration
	minPenetration = std::numeric_limits<float>::max();

	// Iterates over each normal and checks for overlap
	for (const sf::Vector2f& normal : normals)
	{
		// Finds the minimum and maximum vertices projected onto the normal vector
		float leftMin = std::numeric_limits<float>::max(), leftMax = -std::numeric_limits<float>::max();
		for (size_t i = 0; i < leftVertices.getVertexCount(); ++i)
		{
			float projection = leftVertices[i].position.dot(normal);
			leftMin = std::min(leftMin, projection);
			leftMax = std::max(leftMax, projection);
		}

		float rightMin = std::numeric_limits<float>::max(), rightMax = -std::numeric_limits<float>::max();
		for (size_t i = 0; i < rightVertices.getVertexCount(); ++i)
		{
			float projection = rightVertices[i].position.dot(normal);
			rightMin = std::min(projection, rightMin);
			rightMax = std::max(projection, rightMax);
		}

		// If there is overlap, calculate the penetration value and the normal
		if (std::max(leftMin, rightMin) <= std::min(leftMax, rightMax))
		{
			float penetration = std::min(std::fabs(leftMax - rightMin), std::fabs(rightMax - leftMin));
			if (penetration < minPenetration)
			{
				minPenetration = penetration;
				penetrationAxis = (rightMin > leftMin) ? normal : normal * -1.0f;
			}
		}
		else return false;
	}

	return true;
}

#if BUILD_DEBUG
void CollisionPolygon::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	target.draw(m_vertices, states);
}
#endif


// Main function
//int main()
//{
//	// Window
//	sf::RenderWindow window{
//		sf::VideoMode{ { 1200u, 675u } }, "Collision Detection and Resolution using SAT", sf::Style::Close
//	};
//	sf::Clock clock;
//
//	// Player data
//	float playerRadius = 40.0f;
//	int playerVertecies = 4;
//	float maxSpeed = 100.0f;
//	float maxAngularSpeed = 100.0f;
//
//	// Polygons
//	std::vector<CollisionPolygon*> polygons;
//	auto player = new CollisionPolygon{ playerRadius, static_cast<size_t>(playerVertecies), sf::Color::Red };
//	player->setPosition({ 100.0f, 100.0f });
//	polygons.push_back(player);
//	polygons.push_back(new CollisionPolygon{ 50.0f, 4 });
//	polygons.push_back(new CollisionPolygon{ 80.0f, 6 });
//	polygons.at(1)->setPosition({ 400.0f, 200.0f });
//	polygons.at(2)->setPosition({ 700.0f, 300.0f });
//	polygons.at(1)->SetAngularVelocity(100.0f);
//
//	std::vector<std::pair<bool, sf::Vector2f>> polyDrag{ polygons.size(), std::make_pair(false, sf::Vector2f{}) };
//
//	// Main game loop
//	while (window.isOpen())
//	{
//		// Update
//		sf::Time clockRestart = clock.restart();
//		float deltaTime = clockRestart.asSeconds();
//
//		for (CollisionPolygon* polygon : polygons)
//		{
//			polygon->move(polygon->GetVelocity() * deltaTime);
//			{
//				polygon->rotate(sf::radians(polygon->GetAngularVelocity() * deltaTime));
//			}
//		}
//
//		// Collision detection and resolution
//		for (CollisionPolygon* left : polygons)
//		{
//			for (CollisionPolygon* right : polygons)
//			{
//				if (left == right) continue;
//				float minPenetration;
//				sf::Vector2f penetrationAxis;
//				if (detectCollision(left, right, minPenetration, penetrationAxis))
//				{
//					left->move(penetrationAxis * -minPenetration / 2.0f);
//					right->move(penetrationAxis * minPenetration / 2.0f);
//				}
//			}
//		}
//
//		// Render
//		window.clear({ 50, 40, 80 });
//		for (CollisionPolygon* polygon : polygons)
//		{
//			window.draw(*polygon);
//		}
//
//		window.display();
//	}
//
//	return 0;
//}
