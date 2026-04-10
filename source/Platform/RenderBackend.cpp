#if defined(__ANDROID__)
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#else
#include "stdafx.h"
#endif

#include "Platform/RenderBackend.h"
#include "Platform/OpenGLESRenderBackend.h"

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace platform
{
	namespace
	{
		const float kPi = 3.14159265358979323846f;

		struct Matrix4
		{
			float values[16];
		};

		struct SceneState
		{
			Matrix4 projection_matrix;
			std::vector<Matrix4> model_view_stack;
			int viewport_x;
			int viewport_y;
			int viewport_width;
			int viewport_height;
			int viewport_window_height;
		};

		Matrix4 MakeIdentityMatrix()
		{
			Matrix4 matrix = {};
			matrix.values[0] = 1.0f;
			matrix.values[5] = 1.0f;
			matrix.values[10] = 1.0f;
			matrix.values[15] = 1.0f;
			return matrix;
		}

		Matrix4 MultiplyMatrices(const Matrix4& lhs, const Matrix4& rhs)
		{
			Matrix4 result = {};
			for (int column = 0; column < 4; ++column)
			{
				for (int row = 0; row < 4; ++row)
				{
					float value = 0.0f;
					for (int index = 0; index < 4; ++index)
					{
						value += lhs.values[index * 4 + row] * rhs.values[column * 4 + index];
					}
					result.values[column * 4 + row] = value;
				}
			}
			return result;
		}

		Matrix4 MakeTranslationMatrix(float x, float y, float z)
		{
			Matrix4 matrix = MakeIdentityMatrix();
			matrix.values[12] = x;
			matrix.values[13] = y;
			matrix.values[14] = z;
			return matrix;
		}

		Matrix4 MakeRotationMatrix(float angle, float x, float y, float z)
		{
			const float axis_length = sqrtf(x * x + y * y + z * z);
			if (axis_length <= 0.00001f)
			{
				return MakeIdentityMatrix();
			}

			x /= axis_length;
			y /= axis_length;
			z /= axis_length;

			const float radians = angle * kPi / 180.0f;
			const float cosine = cosf(radians);
			const float sine = sinf(radians);
			const float one_minus_cosine = 1.0f - cosine;

			Matrix4 matrix = MakeIdentityMatrix();
			matrix.values[0] = x * x * one_minus_cosine + cosine;
			matrix.values[1] = y * x * one_minus_cosine + z * sine;
			matrix.values[2] = z * x * one_minus_cosine - y * sine;
			matrix.values[4] = x * y * one_minus_cosine - z * sine;
			matrix.values[5] = y * y * one_minus_cosine + cosine;
			matrix.values[6] = z * y * one_minus_cosine + x * sine;
			matrix.values[8] = x * z * one_minus_cosine + y * sine;
			matrix.values[9] = y * z * one_minus_cosine - x * sine;
			matrix.values[10] = z * z * one_minus_cosine + cosine;
			return matrix;
		}

		Matrix4 MakeFrustumMatrix(float left, float right, float bottom, float top, float near_plane, float far_plane)
		{
			Matrix4 matrix = {};
			matrix.values[0] = (2.0f * near_plane) / (right - left);
			matrix.values[5] = (2.0f * near_plane) / (top - bottom);
			matrix.values[8] = (right + left) / (right - left);
			matrix.values[9] = (top + bottom) / (top - bottom);
			matrix.values[10] = -(far_plane + near_plane) / (far_plane - near_plane);
			matrix.values[11] = -1.0f;
			matrix.values[14] = -(2.0f * far_plane * near_plane) / (far_plane - near_plane);
			return matrix;
		}

		Matrix4 MakePerspectiveMatrix(float fov, float aspect, float near_plane, float far_plane)
		{
			const float radians = fov * 0.5f * kPi / 180.0f;
			const float ymax = near_plane * tanf(radians);
			const float xmax = ymax * aspect;
			return MakeFrustumMatrix(-xmax, xmax, -ymax, ymax, near_plane, far_plane);
		}

		Matrix4 MakeOrthoMatrix(float left, float right, float bottom, float top, float near_plane, float far_plane)
		{
			Matrix4 matrix = MakeIdentityMatrix();
			matrix.values[0] = 2.0f / (right - left);
			matrix.values[5] = 2.0f / (top - bottom);
			matrix.values[10] = -2.0f / (far_plane - near_plane);
			matrix.values[12] = -(right + left) / (right - left);
			matrix.values[13] = -(top + bottom) / (top - bottom);
			matrix.values[14] = -(far_plane + near_plane) / (far_plane - near_plane);
			return matrix;
		}

#if !defined(__ANDROID__)
		class LegacyOpenGLRenderBackend : public RenderBackend
		{
		public:
			LegacyOpenGLRenderBackend()
			{
				ResetState(false);
			}

			void ResetState(bool apply_to_gl)
			{
				m_projection_matrix = MakeIdentityMatrix();
				m_model_view_stack.clear();
				m_model_view_stack.push_back(MakeIdentityMatrix());
				m_scene_stack.clear();
				m_viewport_x = 0;
				m_viewport_y = 0;
				m_viewport_width = 0;
				m_viewport_height = 0;
				m_viewport_window_height = 0;

				if (apply_to_gl)
				{
					ApplyMatrices();
				}
			}

			virtual void SetViewport(int x, int y, int width, int height, int window_height)
			{
				m_viewport_x = x;
				m_viewport_y = y;
				m_viewport_width = width;
				m_viewport_height = height;
				m_viewport_window_height = window_height;
				glViewport(x, window_height - (y + height), width, height);
			}

			virtual void SetClearColor(float r, float g, float b, float a)
			{
				glClearColor(r, g, b, a);
			}

			virtual void Clear(unsigned int clear_mask)
			{
				glClear(clear_mask);
			}

			virtual void Flush()
			{
				glFlush();
			}

			virtual bool PresentCurrentFrame()
			{
				HDC current_dc = wglGetCurrentDC();
				if (current_dc == NULL)
				{
					return false;
				}

				return SwapBuffers(current_dc) == TRUE;
			}

			virtual void PushPerspectiveScene(int viewport_x, int viewport_y, int viewport_width, int viewport_height, int window_height,
				float fov, float near_plane, float far_plane, const float camera_position[3], const float camera_angle[3],
				bool camera_top_view, bool fog_enabled, float fog_density, const float fog_color[4])
			{
				SaveCurrentScene();
				SetViewport(viewport_x, viewport_y, viewport_width, viewport_height, window_height);

				const float aspect = viewport_height > 0 ? static_cast<float>(viewport_width) / static_cast<float>(viewport_height) : 1.0f;
				m_projection_matrix = MakePerspectiveMatrix(fov, aspect, near_plane, far_plane);

				m_model_view_stack.clear();
				m_model_view_stack.push_back(MakeIdentityMatrix());
				AppendCurrentModelView(MakeRotationMatrix(camera_angle[1], 0.f, 1.f, 0.f));
				if (camera_top_view == false)
				{
					AppendCurrentModelView(MakeRotationMatrix(camera_angle[0], 1.f, 0.f, 0.f));
				}
				AppendCurrentModelView(MakeRotationMatrix(camera_angle[2], 0.f, 0.f, 1.f));
				AppendCurrentModelView(MakeTranslationMatrix(-camera_position[0], -camera_position[1], -camera_position[2]));
				ApplyMatrices();

				glDisable(GL_ALPHA_TEST);
				glEnable(GL_TEXTURE_2D);
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_CULL_FACE);
				glDepthMask(true);
				glDepthFunc(GL_LEQUAL);
				glAlphaFunc(GL_GREATER, 0.25f);

				if (fog_enabled)
				{
					glEnable(GL_FOG);
					glFogi(GL_FOG_MODE, GL_LINEAR);
					glFogf(GL_FOG_DENSITY, fog_density);
					glFogfv(GL_FOG_COLOR, fog_color);
				}
				else
				{
					glDisable(GL_FOG);
				}
			}

			virtual void PushOrthoScene(int window_width, int window_height)
			{
				PushOrthoSceneViewport(0, 0, window_width, window_height, window_height,
					static_cast<float>(window_width), static_cast<float>(window_height));
			}

			virtual void PushOrthoSceneViewport(int viewport_x, int viewport_y, int viewport_width, int viewport_height, int window_height,
				float ortho_width, float ortho_height)
			{
				SaveCurrentScene();
				SetViewport(viewport_x, viewport_y, viewport_width, viewport_height, window_height);
				m_projection_matrix = MakeOrthoMatrix(0.0f, ortho_width, 0.0f, ortho_height, -1.0f, 1.0f);
				m_model_view_stack.clear();
				m_model_view_stack.push_back(MakeIdentityMatrix());
				ApplyMatrices();
			}

			virtual void PopScene()
			{
				if (m_scene_stack.empty())
				{
					return;
				}

				const SceneState previous_state = m_scene_stack.back();
				m_scene_stack.pop_back();
				m_projection_matrix = previous_state.projection_matrix;
				m_model_view_stack = previous_state.model_view_stack;
				EnsureModelViewStack();
				SetViewport(
					previous_state.viewport_x,
					previous_state.viewport_y,
					previous_state.viewport_width,
					previous_state.viewport_height,
					previous_state.viewport_window_height);
				ApplyMatrices();
			}

			virtual void PushModelView()
			{
				EnsureModelViewStack();
				m_model_view_stack.push_back(CurrentModelView());
				ApplyModelViewMatrix();
			}

			virtual void PushIdentityModelView()
			{
				EnsureModelViewStack();
				m_model_view_stack.push_back(MakeIdentityMatrix());
				ApplyModelViewMatrix();
			}

			virtual void PopModelView()
			{
				EnsureModelViewStack();
				if (m_model_view_stack.size() > 1)
				{
					m_model_view_stack.pop_back();
				}
				else
				{
					m_model_view_stack[0] = MakeIdentityMatrix();
				}
				ApplyModelViewMatrix();
			}

			virtual void LoadIdentityModelView()
			{
				EnsureModelViewStack();
				CurrentModelView() = MakeIdentityMatrix();
				ApplyModelViewMatrix();
			}

			virtual void RotateModelView(float angle, float x, float y, float z)
			{
				EnsureModelViewStack();
				AppendCurrentModelView(MakeRotationMatrix(angle, x, y, z));
				ApplyModelViewMatrix();
			}

			virtual void TranslateModelView(float x, float y, float z)
			{
				EnsureModelViewStack();
				AppendCurrentModelView(MakeTranslationMatrix(x, y, z));
				ApplyModelViewMatrix();
			}

			virtual void CopyModelViewMatrix(float matrix[16]) const
			{
				std::memcpy(matrix, CurrentModelView().values, sizeof(float) * 16);
			}

			virtual void SetDepthTestEnabled(bool enabled)
			{
				SetCapability(GL_DEPTH_TEST, enabled);
			}

			virtual void SetCullFaceEnabled(bool enabled)
			{
				SetCapability(GL_CULL_FACE, enabled);
			}

			virtual void SetDepthMaskEnabled(bool enabled)
			{
				glDepthMask(enabled ? GL_TRUE : GL_FALSE);
			}

			virtual void SetTextureEnabled(bool enabled)
			{
				SetCapability(GL_TEXTURE_2D, enabled);
			}

			virtual void SetAlphaTestEnabled(bool enabled)
			{
				SetCapability(GL_ALPHA_TEST, enabled);
			}

			virtual void SetBlendState(bool enabled, unsigned int src_factor, unsigned int dst_factor)
			{
				SetCapability(GL_BLEND, enabled);
				if (enabled)
				{
					glBlendFunc(src_factor, dst_factor);
				}
			}

			virtual void SetDepthFunction(unsigned int func)
			{
				glDepthFunc(func);
			}

			virtual void SetAlphaFunction(unsigned int func, float ref)
			{
				glAlphaFunc(func, ref);
			}

			virtual void SetFogEnabled(bool enabled)
			{
				SetCapability(GL_FOG, enabled);
			}

			virtual void ConfigureLinearFog(float density, const float color[4])
			{
				glFogi(GL_FOG_MODE, GL_LINEAR);
				glFogf(GL_FOG_DENSITY, density);
				glFogfv(GL_FOG_COLOR, color);
			}

			virtual void ConfigureLinearFogRange(float start_distance, float end_distance, const float color[4])
			{
				glFogi(GL_FOG_MODE, GL_LINEAR);
				glFogf(GL_FOG_START, start_distance);
				glFogf(GL_FOG_END, end_distance);
				glFogfv(GL_FOG_COLOR, color);
			}

			virtual void SetTextureCombineMode(TextureCombineMode mode)
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
					mode == TextureCombineMode_Add ? GL_ADD : GL_MODULATE);
			}

			virtual void SetCurrentColor(float r, float g, float b, float a)
			{
				glColor4f(r, g, b, a);
			}

			virtual void DrawQuad2D(const QuadVertex2D vertices[4], bool textured)
			{
				DrawArrays2D(GL_TRIANGLE_FAN, vertices, 4, textured);
			}

			virtual void DrawTriangleFan2D(const QuadVertex2D* vertices, int vertex_count, bool textured)
			{
				DrawArrays2D(GL_TRIANGLE_FAN, vertices, vertex_count, textured);
			}

			virtual void DrawQuad3D(const Vertex3D vertices[4], bool textured, bool use_vertex_color)
			{
				DrawArrays3D(GL_TRIANGLE_FAN, vertices, 4, textured, use_vertex_color);
			}

			virtual void DrawTriangleList3D(const Vertex3D* vertices, int vertex_count, bool textured, bool use_vertex_color)
			{
				DrawArrays3D(GL_TRIANGLES, vertices, vertex_count, textured, use_vertex_color);
			}

			virtual void DrawLineList3D(const Vertex3D* vertices, int vertex_count, bool use_vertex_color)
			{
				DrawArrays3D(GL_LINES, vertices, vertex_count, false, use_vertex_color);
			}

			virtual void DrawPrimitiveArray3D(unsigned int primitive_type, const float* positions, const float* tex_coords, const float* colors,
				int vertex_count, bool textured, bool use_vertex_color)
			{
				DrawSeparateArrays3D(static_cast<GLenum>(primitive_type), positions, tex_coords, colors, vertex_count, textured, use_vertex_color);
			}

		private:
			void SaveCurrentScene()
			{
				SceneState state;
				state.projection_matrix = m_projection_matrix;
				state.model_view_stack = m_model_view_stack;
				state.viewport_x = m_viewport_x;
				state.viewport_y = m_viewport_y;
				state.viewport_width = m_viewport_width;
				state.viewport_height = m_viewport_height;
				state.viewport_window_height = m_viewport_window_height;
				m_scene_stack.push_back(state);
			}

			void EnsureModelViewStack()
			{
				if (m_model_view_stack.empty())
				{
					m_model_view_stack.push_back(MakeIdentityMatrix());
				}
			}

			Matrix4& CurrentModelView()
			{
				return m_model_view_stack.back();
			}

			const Matrix4& CurrentModelView() const
			{
				return m_model_view_stack.back();
			}

			void AppendCurrentModelView(const Matrix4& transform)
			{
				CurrentModelView() = MultiplyMatrices(CurrentModelView(), transform);
			}

			void ApplyProjectionMatrix() const
			{
				glMatrixMode(GL_PROJECTION);
				glLoadMatrixf(m_projection_matrix.values);
			}

			void ApplyModelViewMatrix() const
			{
				glMatrixMode(GL_MODELVIEW);
				glLoadMatrixf(CurrentModelView().values);
			}

			void ApplyMatrices() const
			{
				ApplyProjectionMatrix();
				ApplyModelViewMatrix();
			}

			static void SetCapability(GLenum capability, bool enabled)
			{
				if (enabled)
				{
					glEnable(capability);
				}
				else
				{
					glDisable(capability);
				}
			}

			static void DrawArrays2D(GLenum primitive_type, const QuadVertex2D* vertices, int vertex_count, bool textured)
			{
				if (vertices == NULL || vertex_count <= 0)
				{
					return;
				}

				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(2, GL_FLOAT, sizeof(QuadVertex2D), &vertices[0].x);

				glEnableClientState(GL_COLOR_ARRAY);
				glColorPointer(4, GL_FLOAT, sizeof(QuadVertex2D), &vertices[0].r);

				if (textured)
				{
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					glTexCoordPointer(2, GL_FLOAT, sizeof(QuadVertex2D), &vertices[0].u);
				}
				else
				{
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				}

				glDrawArrays(primitive_type, 0, static_cast<GLsizei>(vertex_count));

				if (textured)
				{
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				}
				glDisableClientState(GL_COLOR_ARRAY);
				glDisableClientState(GL_VERTEX_ARRAY);
			}

			static void DrawArrays3D(GLenum primitive_type, const Vertex3D* vertices, int vertex_count, bool textured, bool use_vertex_color)
			{
				if (vertices == NULL || vertex_count <= 0)
				{
					return;
				}

				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3, GL_FLOAT, sizeof(Vertex3D), &vertices[0].x);

				if (textured)
				{
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex3D), &vertices[0].u);
				}
				else
				{
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				}

				if (use_vertex_color)
				{
					glEnableClientState(GL_COLOR_ARRAY);
					glColorPointer(4, GL_FLOAT, sizeof(Vertex3D), &vertices[0].r);
				}
				else
				{
					glDisableClientState(GL_COLOR_ARRAY);
				}

				glDrawArrays(primitive_type, 0, static_cast<GLsizei>(vertex_count));

				if (use_vertex_color)
				{
					glDisableClientState(GL_COLOR_ARRAY);
				}
				if (textured)
				{
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				}
				glDisableClientState(GL_VERTEX_ARRAY);
			}

			static void DrawSeparateArrays3D(GLenum primitive_type, const float* positions, const float* tex_coords, const float* colors,
				int vertex_count, bool textured, bool use_vertex_color)
			{
				if (positions == NULL || vertex_count <= 0)
				{
					return;
				}

				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3, GL_FLOAT, 0, positions);

				if (textured && tex_coords != NULL)
				{
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);
				}
				else
				{
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				}

				if (use_vertex_color && colors != NULL)
				{
					glEnableClientState(GL_COLOR_ARRAY);
					glColorPointer(4, GL_FLOAT, 0, colors);
				}
				else
				{
					glDisableClientState(GL_COLOR_ARRAY);
				}

				glDrawArrays(primitive_type, 0, static_cast<GLsizei>(vertex_count));

				if (use_vertex_color && colors != NULL)
				{
					glDisableClientState(GL_COLOR_ARRAY);
				}
				if (textured && tex_coords != NULL)
				{
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				}
				glDisableClientState(GL_VERTEX_ARRAY);
			}

			Matrix4 m_projection_matrix;
			std::vector<Matrix4> m_model_view_stack;
			std::vector<SceneState> m_scene_stack;
			int m_viewport_x;
			int m_viewport_y;
			int m_viewport_width;
			int m_viewport_height;
			int m_viewport_window_height;
		};

#endif
	#if defined(__ANDROID__)
		RenderBackendType g_preferred_backend_type = RenderBackendType_OpenGLES2;
		RenderBackendType g_active_backend_type = RenderBackendType_OpenGLES2;
	#else
		LegacyOpenGLRenderBackend g_legacy_render_backend;
		RenderBackendType g_preferred_backend_type = RenderBackendType_LegacyOpenGL;
		RenderBackendType g_active_backend_type = RenderBackendType_LegacyOpenGL;
	#endif
		RenderBackend* g_render_backend = NULL;

		void ApplyRenderBackendEnvironmentOverride()
		{
			const char* backend_name = getenv("MU_RENDER_BACKEND");
			if (backend_name == NULL)
			{
				return;
			}

			if (strcmp(backend_name, "gles2") == 0 || strcmp(backend_name, "opengles2") == 0)
			{
				g_preferred_backend_type = RenderBackendType_OpenGLES2;
			}
			else if (strcmp(backend_name, "legacy") == 0 || strcmp(backend_name, "gl") == 0)
			{
				g_preferred_backend_type = RenderBackendType_LegacyOpenGL;
			}
		}
	}

	void InitializeRenderBackend()
	{
		ApplyRenderBackendEnvironmentOverride();
		g_render_backend = NULL;
	#if defined(__ANDROID__)
		g_active_backend_type = RenderBackendType_OpenGLES2;
	#else
		g_active_backend_type = RenderBackendType_LegacyOpenGL;
	#endif

		if (g_preferred_backend_type == RenderBackendType_OpenGLES2 && IsOpenGLESRenderBackendAvailable())
		{
			g_render_backend = CreateOpenGLESRenderBackend();
			if (g_render_backend != NULL)
			{
				g_active_backend_type = RenderBackendType_OpenGLES2;
			}
		}

	#if defined(__ANDROID__)
		if (g_render_backend == NULL && IsOpenGLESRenderBackendAvailable())
		{
			g_render_backend = CreateOpenGLESRenderBackend();
			if (g_render_backend != NULL)
			{
				g_active_backend_type = RenderBackendType_OpenGLES2;
			}
		}
	#else
		if (g_render_backend == NULL)
		{
			g_legacy_render_backend.ResetState(wglGetCurrentContext() != NULL);
			g_render_backend = &g_legacy_render_backend;
			g_active_backend_type = RenderBackendType_LegacyOpenGL;
		}
	#endif
	}

	void ShutdownRenderBackend()
	{
		ShutdownOpenGLESRenderBackend();
	#if !defined(__ANDROID__)
		g_legacy_render_backend.ResetState(false);
		g_active_backend_type = RenderBackendType_LegacyOpenGL;
	#else
		g_active_backend_type = RenderBackendType_OpenGLES2;
	#endif
		g_render_backend = NULL;
	}

	RenderBackend& GetRenderBackend()
	{
		if (g_render_backend == NULL)
		{
			InitializeRenderBackend();
		}

		return *g_render_backend;
	}

	void SetPreferredRenderBackendType(RenderBackendType type)
	{
		g_preferred_backend_type = type;
	}

	RenderBackendType GetPreferredRenderBackendType()
	{
		return g_preferred_backend_type;
	}

	RenderBackendType GetActiveRenderBackendType()
	{
		return g_active_backend_type;
	}

	bool IsRenderBackendTypeAvailable(RenderBackendType type)
	{
		switch (type)
		{
		case RenderBackendType_OpenGLES2:
			return IsOpenGLESRenderBackendAvailable();

		case RenderBackendType_LegacyOpenGL:
		default:
		#if defined(__ANDROID__)
			return false;
		#else
			return true;
		#endif
		}
	}

	const char* GetRenderBackendTypeName(RenderBackendType type)
	{
		switch (type)
		{
		case RenderBackendType_OpenGLES2:
			return "OpenGL ES 2.0";

		case RenderBackendType_LegacyOpenGL:
		default:
			return "Legacy OpenGL";
		}
	}
}
