#pragma once

#include "CatEngine/Scene/Components/Physics/Rigidbody2D.h"

#include <glm/glm.hpp>

#include <box2d/b2_body.h>

#include "CatEngine/Scene/Entity.h"

namespace CatEngine
{
	class Physics2D
	{
	public:
		static inline void ApplyForce(Rigidbody2DComponent& rb2d, const glm::vec2& impulse, const glm::vec2& point, bool wake)
		{
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->ApplyForce(b2Vec2(impulse.x, impulse.y), b2Vec2(point.x, point.y), wake);
		}
		static inline void ApplyForceToCenter(Rigidbody2DComponent& rb2d, const glm::vec2& impulse, bool wake)
		{
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->ApplyForceToCenter(b2Vec2(impulse.x, impulse.y), wake);
		}
		static inline void ApplyLinearImpulse(Rigidbody2DComponent& rb2d, const glm::vec2& impulse, const glm::vec2& point, bool wake)
		{
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->ApplyLinearImpulse(b2Vec2(impulse.x, impulse.y), b2Vec2(point.x, point.y), wake);
		}
		static inline void ApplyLinearImpulseToCenter(Rigidbody2DComponent& rb2d, const glm::vec2& impulse, bool wake)
		{
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->ApplyLinearImpulseToCenter(b2Vec2(impulse.x, impulse.y), wake);
		}
	};
}
