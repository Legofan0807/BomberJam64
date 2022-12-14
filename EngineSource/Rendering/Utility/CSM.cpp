#include "CSM.h"
#include <Rendering/Camera/Camera.h>
#include <World/Graphics.h>
#include <iostream>
#include <cmath>

namespace CSM
{
	const float cameraFarPlane = 600.f;
	std::vector<float> shadowCascadeLevels{ cameraFarPlane / 12.f, cameraFarPlane / 4.f, cameraFarPlane / 1.f };
	GLuint LightFBO;
	int Cascades = 3;
	GLuint ShadowMaps;
	bool Initialized = false;
	std::vector<glm::vec4> CSM::getFrustumCornersWorldSpace(const glm::mat4& projview)
	{
		const auto inv = glm::inverse(projview);

		std::vector<glm::vec4> frustumCorners;
		for (unsigned int x = 0; x < 2; ++x)
		{
			for (unsigned int y = 0; y < 2; ++y)
			{
				for (unsigned int z = 0; z < 2; ++z)
				{
					const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
					frustumCorners.push_back(pt / pt.w);
				}
			}
		}

		return frustumCorners;
	}

	std::string CSM::ErrorMessageFromGLStatus(int Status)
	{
		//https://neslib.github.io/Ooogles.Net/html/0e1349ae-da69-6e5e-edd6-edd8523101f8.htm

		switch (Status)
		{
		case 36053:
			return "The framebuffer is complete\n";
		case 36054:
			return "Not all framebuffer attachment points are framebuffer attachment complete.\nThis means that at least one attachment point with a renderbuffer or texture attached has its attached object no longer in existence \nor has an attached image with a width or height of zero, or the color attachment point has a non-color-renderable image attached, \nor the depth attachment point has a non-depth-renderable image attached, or the stencil attachment point \nhas a non-stencil-renderable image attached.\n";
		case 36057:
			return "Not all attached images have the same width and height.";
		case 36055:
			return "No images are attached to the framebuffer.";
		case 36061:
			return "The combination of internal formats of the attached images violates an implementation-dependent set of restrictions.";
		}
		return "Error: The given Status is not A glFramebufferError";
	}

	std::vector<glm::vec4> CSM::getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
	{
		return getFrustumCornersWorldSpace(proj * view);
	}

	void CSM::Init()
	{
		Initialized = true;
		glGenFramebuffers(1, &LightFBO);

		glGenTextures(1, &ShadowMaps);
		glBindTexture(GL_TEXTURE_2D_ARRAY, ShadowMaps);
		glTexImage3D(
			GL_TEXTURE_2D_ARRAY,
			0,
			GL_DEPTH_COMPONENT32F,
			Graphics::ShadowResolution,
			Graphics::ShadowResolution,
			int(shadowCascadeLevels.size()) + 1,
			0,
			GL_DEPTH_COMPONENT,
			GL_FLOAT,
			nullptr);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

		glBindFramebuffer(GL_FRAMEBUFFER, LightFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, ShadowMaps, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
			throw 0;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void CSM::ReInit()
	{
		if (Initialized)
		{
			glDeleteTextures(1, &ShadowMaps);
			glDeleteFramebuffers(1, &LightFBO);
			glGenFramebuffers(1, &LightFBO);

			glGenTextures(1, &ShadowMaps);
			glBindTexture(GL_TEXTURE_2D_ARRAY, ShadowMaps);
			glTexImage3D(
				GL_TEXTURE_2D_ARRAY,
				0,
				GL_DEPTH_COMPONENT32F,
				Graphics::ShadowResolution,
				Graphics::ShadowResolution,
				int(shadowCascadeLevels.size()) + 1,
				0,
				GL_DEPTH_COMPONENT,
				GL_FLOAT,
				nullptr);

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

			glBindFramebuffer(GL_FRAMEBUFFER, LightFBO);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, ShadowMaps, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

			int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE)
			{
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
				throw 0;
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	glm::mat4 CSM::getLightSpaceMatrix(const float nearPlane, const float farPlane)
	{
		const auto proj = glm::perspective(
			2.f, Graphics::WindowResolution.X / Graphics::WindowResolution.Y, nearPlane,
			farPlane);
		const auto corners = getFrustumCornersWorldSpace(proj, Graphics::MainCamera->getView());

		glm::vec3 center = glm::vec3(0, 0, 0);
		for (const auto& v : corners)
		{
			center += glm::vec3(v);
		}
		center /= corners.size();

		const auto lightView = glm::lookAt(center + (glm::vec3)Graphics::LightRotation, center, glm::vec3(0.0f, 1.0f, 0.0f));

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::min();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::min();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::min();
		for (const auto& v : corners)
		{
			const auto trf = lightView * v;
			minX = std::min(minX, trf.x);
			maxX = std::max(maxX, trf.x);
			minY = std::min(minY, trf.y);
			maxY = std::max(maxY, trf.y);
			minZ = std::min(minZ, trf.z);
			maxZ = std::max(maxZ, trf.z);
		}

		// Tune this parameter according to the scene
		constexpr float zMult = 25;
		if (minZ < 0)
		{
			minZ *= zMult;
		}
		else
		{
			minZ /= zMult;
		}
		if (maxZ < 0)
		{
			maxZ /= zMult;
		}
		else
		{
			maxZ *= zMult;
		}

		float zoom = 1;

		const glm::mat4 lightProjection = glm::ortho(minX / zoom, maxX / zoom, minY / zoom, maxY / zoom, minZ / zoom, maxZ / zoom);

		return lightProjection * lightView;
	}

	std::vector<glm::mat4> CSM::getLightSpaceMatrices()
	{
		std::vector<glm::mat4> ret;
		for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
		{
			if (i == 0)
			{
				ret.push_back(getLightSpaceMatrix(0.1f, shadowCascadeLevels[i]));
			}
			else if (i < shadowCascadeLevels.size())
			{
				ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
			}
			else
			{
				ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], cameraFarPlane));
			}
		}
		return ret;
	}
}