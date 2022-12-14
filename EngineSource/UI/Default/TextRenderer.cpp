#define _CRT_SECURE_NO_WARNINGS
#include "TextRenderer.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include <Utility/stb_truetype.h>
#include <Engine/FileUtility.h>
#include <UI/Default/ScrollObject.h>
#include <Engine/Log.h>
#include <Rendering/Shader.h>
#include <SDL.h>
#include <World/Stats.h>
#include <World/Graphics.h>

std::vector<TextRenderer*> Renderers;
TextRenderer::TextRenderer(std::string filename, float CharacterSizeInPixels)
{
	filename = GetFileNameFromPath(filename);
	Renderers.push_back(this);
	stbtt_bakedchar* cdata = new stbtt_bakedchar[96];
	Uint8* ttfBuffer = (Uint8*)malloc(1 << 20);
	Uint8* tmpBitmap = new Uint8[512 * 512];
	if (IsInEditor || EngineDebug)
	{
		filename = "Fonts/" + filename;
	}
	else
	{
		filename = "Assets/" + filename;
	}
	Filename = filename;
	this->CharacterSizeInPixels = CharacterSizeInPixels;
	fread(ttfBuffer, 1, 1 << 20, fopen(Filename.c_str(), "rb"));
	stbtt_BakeFontBitmap(ttfBuffer, 0, CharacterSizeInPixels, tmpBitmap, 512, 512, 32, 96, cdata); // no guarantee this fits!
	// can free ttf_buffer at this point
	glGenTextures(1, &fontTexture);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, tmpBitmap);
	// can free temp_bitmap at this point
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenVertexArrays(1, &fontVao);
	glBindVertexArray(fontVao);
	glGenBuffers(1, &fontVertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, fontVertexBufferId);
	fontVertexBufferCapacity = 20;
	fontVertexBufferData = new FontVertex[fontVertexBufferCapacity * 6];
	glBufferData(GL_ARRAY_BUFFER, sizeof(FontVertex) * 6 * fontVertexBufferCapacity, 0, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(FontVertex), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(FontVertex), (const void*)offsetof(FontVertex, texCoords));
	glBindVertexArray(0);
	cdatapointer = cdata;
	free(ttfBuffer);
	delete[] tmpBitmap;
}

void TextRenderer::RenderText(std::string Text, Vector2 Pos, float scale, Vector3 color, ScrollObject* CurrentScrollObject)
{
	scale /= 2.f;
	Pos.X = Pos.X * 800;
	Pos.Y = Pos.Y * -450;
	Graphics::TextShader->Bind();
	stbtt_bakedchar* cdata = (stbtt_bakedchar*)cdatapointer;
	const char* text = Text.c_str();
	glBindVertexArray(fontVao);
	glBindBuffer(GL_ARRAY_BUFFER, fontVertexBufferId);
	Uint32 len = strlen(Text.c_str());
	if (fontVertexBufferCapacity < len) {
		fontVertexBufferCapacity = len;
		glBufferData(GL_ARRAY_BUFFER, sizeof(FontVertex) * 6 * fontVertexBufferCapacity, 0, GL_DYNAMIC_DRAW);
		delete[]fontVertexBufferData;
		fontVertexBufferData = new FontVertex[fontVertexBufferCapacity * 6];
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glUniform1i(glGetUniformLocation(Graphics::TextShader->GetShaderID(), "u_texture"), 0);
	glUniform3f(glGetUniformLocation(Graphics::TextShader->GetShaderID(), "textColor"), color.X, color.Y, color.Z);
	glUniformMatrix4fv(glGetUniformLocation(Graphics::TextShader->GetShaderID(), "projection"), 1, false, &projection[0][0]);
	glUniform3f(glGetUniformLocation(Graphics::TextShader->GetShaderID(), "transform"), Pos.X, Pos.Y, scale);
	if (CurrentScrollObject != nullptr)
	{
		glUniform3f(glGetUniformLocation(Graphics::TextShader->GetShaderID(), "u_offset"), -CurrentScrollObject->Percentage, CurrentScrollObject->Position.Y + CurrentScrollObject->Scale.Y, CurrentScrollObject->Position.Y);
	}
	else
		glUniform3f(glGetUniformLocation(Graphics::TextShader->GetShaderID(), "u_offset"), 0, -1000, 1000);
	float x, y = 0;
	FontVertex* vData = fontVertexBufferData;
	Uint32 numVertices = 0;
	while (*text) {
		if (*text >= 32 && *text < 128) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(cdata, 512, 512, *text - 32, &x, &y, &q, 1);
			vData[0].position = glm::vec2(q.x0, q.y1); vData[0].texCoords = glm::vec2(q.s0, q.t1);
			vData[1].position = glm::vec2(q.x1, q.y1); vData[1].texCoords = glm::vec2(q.s1, q.t1);
			vData[2].position = glm::vec2(q.x1, q.y0); vData[2].texCoords = glm::vec2(q.s1, q.t0);
			vData[3].position = glm::vec2(q.x0, q.y0); vData[3].texCoords = glm::vec2(q.s0, q.t0);
			vData[4].position = glm::vec2(q.x0, q.y1); vData[4].texCoords = glm::vec2(q.s0, q.t1);
			vData[5].position = glm::vec2(q.x1, q.y0); vData[5].texCoords = glm::vec2(q.s1, q.t0);
			vData += 6;
			numVertices += 6;
		}
		++text;
	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(FontVertex) * numVertices, fontVertexBufferData);
	glDrawArrays(GL_TRIANGLES, 0, numVertices);
	Graphics::UIShader->Bind();
}

void TextRenderer::Reinit()
{
	if (fontVertexBufferData)
	{
		delete[]fontVertexBufferData;
	}
	delete[] cdatapointer;
	stbtt_bakedchar* cdata = new stbtt_bakedchar[96];
	Uint8* ttfBuffer = (Uint8*)malloc(1 << 20);
	Uint8* tmpBitmap = new Uint8[512 * 512];
	fread(ttfBuffer, 1, 1 << 20, fopen("Fonts/Font.ttf", "rb"));
	stbtt_BakeFontBitmap(ttfBuffer, 0, CharacterSizeInPixels, tmpBitmap, 512, 512, 32, 96, cdata); // no guarantee this fits!
	// can free ttf_buffer at this point
	glGenTextures(1, &fontTexture);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, tmpBitmap);
	// can free temp_bitmap at this point
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenVertexArrays(1, &fontVao);
	glBindVertexArray(fontVao);
	glGenBuffers(1, &fontVertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, fontVertexBufferId);
	fontVertexBufferCapacity = 20;
	fontVertexBufferData = new FontVertex[fontVertexBufferCapacity * 6];
	glBufferData(GL_ARRAY_BUFFER, sizeof(FontVertex) * 6 * fontVertexBufferCapacity, 0, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(FontVertex), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(FontVertex), (const void*)offsetof(FontVertex, texCoords));
	glBindVertexArray(0);
	cdatapointer = cdata;
	free(ttfBuffer);
	delete[] tmpBitmap;
}

TextRenderer::~TextRenderer()
{
	unsigned int i = 0;
	for (TextRenderer* r : Renderers)
	{
		if (r == this)
			Renderers.erase(Renderers.begin() + i);
		i++;
	}
	glDeleteTextures(1, &fontTexture);
	glDeleteBuffers(1, &fontVertexBufferId);
	glDeleteBuffers(1, &fontVao);
	if (fontVertexBufferData)
	{
		delete[]fontVertexBufferData;
	}
	delete[] cdatapointer;
}

void OnWindowResized()
{
}
