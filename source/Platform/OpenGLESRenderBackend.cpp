#if defined(__ANDROID__)
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#else
#include "stdafx.h"
#endif

#include "Platform/OpenGLESRenderBackend.h"

#include <cmath>
#include <cstring>
#include <vector>

namespace platform
{
	namespace
	{
		const float kPi = 3.14159265358979323846f;
		const GLuint kPositionAttributeIndex = 0;
		const GLuint kTexCoordAttributeIndex = 1;
		const GLuint kColorAttributeIndex = 2;

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

		struct ShaderVertex
		{
			float x;
			float y;
			float z;
			float u;
			float v;
			float r;
			float g;
			float b;
			float a;
		};

		struct ShaderProgramState
		{
			GLuint program;
			GLuint vertex_shader;
			GLuint fragment_shader;
			GLint projection_matrix_uniform;
			GLint model_view_matrix_uniform;
			GLint texture_sampler_uniform;
			GLint current_color_uniform;
			GLint alpha_ref_uniform;
			GLint texture_enabled_uniform;
			GLint use_vertex_color_uniform;
			GLint alpha_test_enabled_uniform;
			GLint fog_enabled_uniform;
			GLint fog_mode_uniform;
			GLint fog_density_uniform;
			GLint fog_start_uniform;
			GLint fog_end_uniform;
			GLint texture_combine_mode_uniform;
			GLint fog_color_uniform;
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

		void SetShaderVertex(ShaderVertex& target, float x, float y, float z, float u, float v, float r, float g, float b, float a)
		{
			target.x = x;
			target.y = y;
			target.z = z;
			target.u = u;
			target.v = v;
			target.r = r;
			target.g = g;
			target.b = b;
			target.a = a;
		}

		bool IsCurrentShaderContextAvailable()
		{
		#if defined(__ANDROID__)
			return eglGetCurrentContext() != EGL_NO_CONTEXT;
		#else
			return wglGetCurrentContext() != NULL;
		#endif
		}

		bool PresentCurrentShaderFrame()
		{
		#if defined(__ANDROID__)
			EGLDisplay current_display = eglGetCurrentDisplay();
			EGLSurface current_surface = eglGetCurrentSurface(EGL_DRAW);
			if (current_display == EGL_NO_DISPLAY || current_surface == EGL_NO_SURFACE)
			{
				return false;
			}

			return eglSwapBuffers(current_display, current_surface) == EGL_TRUE;
		#else
			HDC current_dc = wglGetCurrentDC();
			if (current_dc == NULL)
			{
				return false;
			}

			return SwapBuffers(current_dc) == TRUE;
		#endif
		}

		bool IsShaderPipelineAvailable()
		{
		#if defined(__ANDROID__)
			return IsCurrentShaderContextAvailable();
		#else
			return IsCurrentShaderContextAvailable()
				&& glCreateShader != NULL
				&& glShaderSource != NULL
				&& glCompileShader != NULL
				&& glGetShaderiv != NULL
				&& glDeleteShader != NULL
				&& glCreateProgram != NULL
				&& glAttachShader != NULL
				&& glBindAttribLocation != NULL
				&& glLinkProgram != NULL
				&& glGetProgramiv != NULL
				&& glDeleteProgram != NULL
				&& glUseProgram != NULL
				&& glGetUniformLocation != NULL
				&& glUniformMatrix4fv != NULL
				&& glUniform1i != NULL
				&& glUniform1f != NULL
				&& glUniform4fv != NULL
				&& glEnableVertexAttribArray != NULL
				&& glDisableVertexAttribArray != NULL
				&& glVertexAttribPointer != NULL
				&& glVertexAttrib2f != NULL
				&& glVertexAttrib4f != NULL;
		#endif
		}

		bool CompileShader(GLenum shader_type, const char* source, GLuint& shader)
		{
			shader = glCreateShader(shader_type);
			if (shader == 0)
			{
				return false;
			}

			glShaderSource(shader, 1, &source, NULL);
			glCompileShader(shader);

			GLint compile_status = GL_FALSE;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
			if (compile_status != GL_TRUE)
			{
				glDeleteShader(shader);
				shader = 0;
				return false;
			}

			return true;
		}

		bool LinkProgram(GLuint vertex_shader, GLuint fragment_shader, GLuint& program)
		{
			program = glCreateProgram();
			if (program == 0)
			{
				return false;
			}

			glAttachShader(program, vertex_shader);
			glAttachShader(program, fragment_shader);
			glBindAttribLocation(program, kPositionAttributeIndex, "a_position");
			glBindAttribLocation(program, kTexCoordAttributeIndex, "a_tex_coord");
			glBindAttribLocation(program, kColorAttributeIndex, "a_color");
			glLinkProgram(program);

			GLint link_status = GL_FALSE;
			glGetProgramiv(program, GL_LINK_STATUS, &link_status);
			if (link_status != GL_TRUE)
			{
				glDeleteProgram(program);
				program = 0;
				return false;
			}

			return true;
		}

		class OpenGLES2RenderBackend : public RenderBackend
		{
		public:
			OpenGLES2RenderBackend()
			{
				m_shader_program.program = 0;
				m_shader_program.vertex_shader = 0;
				m_shader_program.fragment_shader = 0;
				m_initialized = false;
				ResetState();
			}

			bool Initialize()
			{
				ResetState();
				if (m_initialized)
				{
					return true;
				}

				if (!IsShaderPipelineAvailable())
				{
					return false;
				}

				static const char* kVertexShaderSource =
					"attribute vec3 a_position;\n"
					"attribute vec2 a_tex_coord;\n"
					"attribute vec4 a_color;\n"
					"uniform mat4 u_projection;\n"
					"uniform mat4 u_model_view;\n"
					"varying vec2 v_tex_coord;\n"
					"varying vec4 v_color;\n"
					"varying float v_eye_depth;\n"
					"void main()\n"
					"{\n"
					"    vec4 eye_position = u_model_view * vec4(a_position, 1.0);\n"
					"    gl_Position = u_projection * eye_position;\n"
					"    v_tex_coord = a_tex_coord;\n"
					"    v_color = a_color;\n"
					"    v_eye_depth = abs(eye_position.z);\n"
					"}\n";

				static const char* kFragmentShaderSource =
					"#ifdef GL_ES\n"
					"precision mediump float;\n"
					"#endif\n"
					"uniform sampler2D u_texture;\n"
					"uniform vec4 u_current_color;\n"
					"uniform float u_alpha_ref;\n"
					"uniform float u_texture_enabled;\n"
					"uniform float u_use_vertex_color;\n"
					"uniform float u_alpha_test_enabled;\n"
					"uniform float u_fog_enabled;\n"
					"uniform float u_fog_mode;\n"
					"uniform float u_fog_density;\n"
					"uniform float u_fog_start;\n"
					"uniform float u_fog_end;\n"
					"uniform float u_texture_combine_mode;\n"
					"uniform vec4 u_fog_color;\n"
					"varying vec2 v_tex_coord;\n"
					"varying vec4 v_color;\n"
					"varying float v_eye_depth;\n"
					"void main()\n"
					"{\n"
					"    vec4 color = mix(u_current_color, v_color, clamp(u_use_vertex_color, 0.0, 1.0));\n"
					"    if (u_texture_enabled > 0.5)\n"
					"    {\n"
					"        vec4 texture_color = texture2D(u_texture, v_tex_coord);\n"
					"        if (u_texture_combine_mode > 0.5)\n"
					"        {\n"
					"            color.rgb = min(color.rgb + texture_color.rgb, vec3(1.0));\n"
					"            color.a *= texture_color.a;\n"
					"        }\n"
					"        else\n"
					"        {\n"
					"            color *= texture_color;\n"
					"        }\n"
					"    }\n"
					"    if (u_alpha_test_enabled > 0.5 && color.a <= u_alpha_ref)\n"
					"    {\n"
					"        discard;\n"
					"    }\n"
					"    if (u_fog_enabled > 0.5)\n"
					"    {\n"
					"        float fog_factor = 1.0;\n"
					"        if (u_fog_mode > 0.5)\n"
					"        {\n"
					"            float fog_range = max(u_fog_end - u_fog_start, 0.0001);\n"
					"            fog_factor = clamp((u_fog_end - v_eye_depth) / fog_range, 0.0, 1.0);\n"
					"        }\n"
					"        else\n"
					"        {\n"
					"            fog_factor = clamp(exp(-max(u_fog_density, 0.000001) * v_eye_depth), 0.0, 1.0);\n"
					"        }\n"
					"        color.rgb = mix(u_fog_color.rgb, color.rgb, fog_factor);\n"
					"    }\n"
					"    gl_FragColor = color;\n"
					"}\n";

				if (!CompileShader(GL_VERTEX_SHADER, kVertexShaderSource, m_shader_program.vertex_shader))
				{
					Shutdown();
					return false;
				}

				if (!CompileShader(GL_FRAGMENT_SHADER, kFragmentShaderSource, m_shader_program.fragment_shader))
				{
					Shutdown();
					return false;
				}

				if (!LinkProgram(m_shader_program.vertex_shader, m_shader_program.fragment_shader, m_shader_program.program))
				{
					Shutdown();
					return false;
				}

				m_shader_program.projection_matrix_uniform = glGetUniformLocation(m_shader_program.program, "u_projection");
				m_shader_program.model_view_matrix_uniform = glGetUniformLocation(m_shader_program.program, "u_model_view");
				m_shader_program.texture_sampler_uniform = glGetUniformLocation(m_shader_program.program, "u_texture");
				m_shader_program.current_color_uniform = glGetUniformLocation(m_shader_program.program, "u_current_color");
				m_shader_program.alpha_ref_uniform = glGetUniformLocation(m_shader_program.program, "u_alpha_ref");
				m_shader_program.texture_enabled_uniform = glGetUniformLocation(m_shader_program.program, "u_texture_enabled");
				m_shader_program.use_vertex_color_uniform = glGetUniformLocation(m_shader_program.program, "u_use_vertex_color");
				m_shader_program.alpha_test_enabled_uniform = glGetUniformLocation(m_shader_program.program, "u_alpha_test_enabled");
				m_shader_program.fog_enabled_uniform = glGetUniformLocation(m_shader_program.program, "u_fog_enabled");
				m_shader_program.fog_mode_uniform = glGetUniformLocation(m_shader_program.program, "u_fog_mode");
				m_shader_program.fog_density_uniform = glGetUniformLocation(m_shader_program.program, "u_fog_density");
				m_shader_program.fog_start_uniform = glGetUniformLocation(m_shader_program.program, "u_fog_start");
				m_shader_program.fog_end_uniform = glGetUniformLocation(m_shader_program.program, "u_fog_end");
				m_shader_program.texture_combine_mode_uniform = glGetUniformLocation(m_shader_program.program, "u_texture_combine_mode");
				m_shader_program.fog_color_uniform = glGetUniformLocation(m_shader_program.program, "u_fog_color");
				m_initialized = true;
				return true;
			}

			void Shutdown()
			{
				if (IsCurrentShaderContextAvailable())
				{
					glUseProgram(0);
					if (m_shader_program.program != 0)
					{
						glDeleteProgram(m_shader_program.program);
					}
					if (m_shader_program.vertex_shader != 0)
					{
						glDeleteShader(m_shader_program.vertex_shader);
					}
					if (m_shader_program.fragment_shader != 0)
					{
						glDeleteShader(m_shader_program.fragment_shader);
					}
				}

				m_shader_program.program = 0;
				m_shader_program.vertex_shader = 0;
				m_shader_program.fragment_shader = 0;
				m_initialized = false;
				ResetState();
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
				return PresentCurrentShaderFrame();
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

				SetDepthTestEnabled(true);
				SetCullFaceEnabled(true);
				SetDepthMaskEnabled(true);
				SetTextureEnabled(true);
				SetAlphaTestEnabled(false);
				SetBlendState(false, GL_ONE, GL_ZERO);
				SetDepthFunction(GL_LEQUAL);
				SetAlphaFunction(GL_GREATER, 0.25f);
				SetFogEnabled(fog_enabled);
				ConfigureLinearFog(fog_density, fog_color);
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
			}

			virtual void PushModelView()
			{
				EnsureModelViewStack();
				m_model_view_stack.push_back(CurrentModelView());
			}

			virtual void PushIdentityModelView()
			{
				EnsureModelViewStack();
				m_model_view_stack.push_back(MakeIdentityMatrix());
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
			}

			virtual void LoadIdentityModelView()
			{
				EnsureModelViewStack();
				CurrentModelView() = MakeIdentityMatrix();
			}

			virtual void RotateModelView(float angle, float x, float y, float z)
			{
				EnsureModelViewStack();
				AppendCurrentModelView(MakeRotationMatrix(angle, x, y, z));
			}

			virtual void TranslateModelView(float x, float y, float z)
			{
				EnsureModelViewStack();
				AppendCurrentModelView(MakeTranslationMatrix(x, y, z));
			}

			virtual void CopyModelViewMatrix(float matrix[16]) const
			{
				std::memcpy(matrix, CurrentModelView().values, sizeof(float) * 16);
			}

			virtual void SetDepthTestEnabled(bool enabled)
			{
				m_depth_test_enabled = enabled;
				SetCapability(GL_DEPTH_TEST, enabled);
			}

			virtual void SetCullFaceEnabled(bool enabled)
			{
				m_cull_face_enabled = enabled;
				SetCapability(GL_CULL_FACE, enabled);
			}

			virtual void SetDepthMaskEnabled(bool enabled)
			{
				m_depth_mask_enabled = enabled;
				glDepthMask(enabled ? GL_TRUE : GL_FALSE);
			}

			virtual void SetTextureEnabled(bool enabled)
			{
				m_texture_enabled = enabled;
			}

			virtual void SetAlphaTestEnabled(bool enabled)
			{
				m_alpha_test_enabled = enabled;
			}

			virtual void SetBlendState(bool enabled, unsigned int src_factor, unsigned int dst_factor)
			{
				m_blend_enabled = enabled;
				m_blend_src_factor = src_factor;
				m_blend_dst_factor = dst_factor;
				SetCapability(GL_BLEND, enabled);
				if (enabled)
				{
					glBlendFunc(src_factor, dst_factor);
				}
			}

			virtual void SetDepthFunction(unsigned int func)
			{
				m_depth_function = func;
				glDepthFunc(func);
			}

			virtual void SetAlphaFunction(unsigned int func, float ref)
			{
				m_alpha_function = func;
				m_alpha_reference = ref;
			}

			virtual void SetFogEnabled(bool enabled)
			{
				m_fog_enabled = enabled;
			}

			virtual void ConfigureLinearFog(float density, const float color[4])
			{
				m_fog_use_range = false;
				m_fog_density = density;
				std::memcpy(m_fog_color, color, sizeof(float) * 4);
			}

			virtual void ConfigureLinearFogRange(float start_distance, float end_distance, const float color[4])
			{
				m_fog_use_range = true;
				m_fog_start = start_distance;
				m_fog_end = end_distance;
				std::memcpy(m_fog_color, color, sizeof(float) * 4);
			}

			virtual void SetTextureCombineMode(TextureCombineMode mode)
			{
				m_texture_combine_mode = mode;
			}

			virtual void SetCurrentColor(float r, float g, float b, float a)
			{
				m_current_color[0] = r;
				m_current_color[1] = g;
				m_current_color[2] = b;
				m_current_color[3] = a;
			}

			virtual void DrawQuad2D(const QuadVertex2D vertices[4], bool textured)
			{
				ShaderVertex shader_vertices[4];
				for (int i = 0; i < 4; ++i)
				{
					SetShaderVertex(shader_vertices[i], vertices[i].x, vertices[i].y, 0.0f, vertices[i].u, vertices[i].v,
						vertices[i].r, vertices[i].g, vertices[i].b, vertices[i].a);
				}
				DrawShaderVertices(GL_TRIANGLE_FAN, shader_vertices, 4, textured, true);
			}

			virtual void DrawTriangleFan2D(const QuadVertex2D* vertices, int vertex_count, bool textured)
			{
				if (vertices == NULL || vertex_count <= 0)
				{
					return;
				}

				std::vector<ShaderVertex> shader_vertices(static_cast<size_t>(vertex_count));
				for (int i = 0; i < vertex_count; ++i)
				{
					SetShaderVertex(shader_vertices[static_cast<size_t>(i)], vertices[i].x, vertices[i].y, 0.0f, vertices[i].u, vertices[i].v,
						vertices[i].r, vertices[i].g, vertices[i].b, vertices[i].a);
				}

				DrawShaderVertices(GL_TRIANGLE_FAN, &shader_vertices[0], vertex_count, textured, true);
			}

			virtual void DrawQuad3D(const Vertex3D vertices[4], bool textured, bool use_vertex_color)
			{
				DrawVertex3D(GL_TRIANGLE_FAN, vertices, 4, textured, use_vertex_color);
			}

			virtual void DrawTriangleList3D(const Vertex3D* vertices, int vertex_count, bool textured, bool use_vertex_color)
			{
				DrawVertex3D(GL_TRIANGLES, vertices, vertex_count, textured, use_vertex_color);
			}

			virtual void DrawLineList3D(const Vertex3D* vertices, int vertex_count, bool use_vertex_color)
			{
				DrawVertex3D(GL_LINES, vertices, vertex_count, false, use_vertex_color);
			}

			virtual void DrawPrimitiveArray3D(unsigned int primitive_type, const float* positions, const float* tex_coords, const float* colors,
				int vertex_count, bool textured, bool use_vertex_color)
			{
				DrawSeparateVertexArrays(static_cast<GLenum>(primitive_type), positions, tex_coords, colors, vertex_count, textured, use_vertex_color);
			}

		private:
			void ResetState()
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
				m_depth_test_enabled = false;
				m_cull_face_enabled = false;
				m_depth_mask_enabled = true;
				m_texture_enabled = true;
				m_alpha_test_enabled = false;
				m_blend_enabled = false;
				m_blend_src_factor = GL_ONE;
				m_blend_dst_factor = GL_ZERO;
				m_depth_function = GL_LEQUAL;
				m_alpha_function = GL_GREATER;
				m_alpha_reference = 0.25f;
				m_fog_enabled = false;
				m_fog_use_range = false;
				m_fog_density = 0.0004f;
				m_fog_start = 2000.0f;
				m_fog_end = 2700.0f;
				m_texture_combine_mode = TextureCombineMode_Modulate;
				m_fog_color[0] = 30 / 256.f;
				m_fog_color[1] = 20 / 256.f;
				m_fog_color[2] = 10 / 256.f;
				m_fog_color[3] = 0.0f;
				m_current_color[0] = 1.0f;
				m_current_color[1] = 1.0f;
				m_current_color[2] = 1.0f;
				m_current_color[3] = 1.0f;
			}

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

			bool PrepareDraw(bool textured, bool use_vertex_color)
			{
				if (!m_initialized && !Initialize())
				{
					return false;
				}

				glUseProgram(m_shader_program.program);

				SetCapability(GL_DEPTH_TEST, m_depth_test_enabled);
				SetCapability(GL_CULL_FACE, m_cull_face_enabled);
				SetCapability(GL_BLEND, m_blend_enabled);
				glDepthMask(m_depth_mask_enabled ? GL_TRUE : GL_FALSE);
				glDepthFunc(m_depth_function);
				if (m_blend_enabled)
				{
					glBlendFunc(m_blend_src_factor, m_blend_dst_factor);
				}

				glUniformMatrix4fv(m_shader_program.projection_matrix_uniform, 1, GL_FALSE, m_projection_matrix.values);
				glUniformMatrix4fv(m_shader_program.model_view_matrix_uniform, 1, GL_FALSE, CurrentModelView().values);
				glUniform1i(m_shader_program.texture_sampler_uniform, 0);
				glUniform4fv(m_shader_program.current_color_uniform, 1, m_current_color);
				glUniform1f(m_shader_program.alpha_ref_uniform, m_alpha_reference);
				glUniform1f(m_shader_program.texture_enabled_uniform, (textured && m_texture_enabled) ? 1.0f : 0.0f);
				glUniform1f(m_shader_program.use_vertex_color_uniform, use_vertex_color ? 1.0f : 0.0f);
				glUniform1f(m_shader_program.alpha_test_enabled_uniform,
					(m_alpha_test_enabled && m_alpha_function == GL_GREATER) ? 1.0f : 0.0f);
				glUniform1f(m_shader_program.fog_enabled_uniform, m_fog_enabled ? 1.0f : 0.0f);
				glUniform1f(m_shader_program.fog_mode_uniform, m_fog_use_range ? 1.0f : 0.0f);
				glUniform1f(m_shader_program.fog_density_uniform, m_fog_density);
				glUniform1f(m_shader_program.fog_start_uniform, m_fog_start);
				glUniform1f(m_shader_program.fog_end_uniform, m_fog_end);
				glUniform1f(m_shader_program.texture_combine_mode_uniform,
					m_texture_combine_mode == TextureCombineMode_Add ? 1.0f : 0.0f);
				glUniform4fv(m_shader_program.fog_color_uniform, 1, m_fog_color);
				return true;
			}

			void DrawShaderVertices(GLenum primitive_type, const ShaderVertex* vertices, int vertex_count, bool textured, bool use_vertex_color)
			{
				if (vertices == NULL || vertex_count <= 0 || !PrepareDraw(textured, use_vertex_color))
				{
					return;
				}

				glEnableVertexAttribArray(kPositionAttributeIndex);
				glVertexAttribPointer(kPositionAttributeIndex, 3, GL_FLOAT, GL_FALSE, sizeof(ShaderVertex), &vertices[0].x);

				if (textured && m_texture_enabled)
				{
					glEnableVertexAttribArray(kTexCoordAttributeIndex);
					glVertexAttribPointer(kTexCoordAttributeIndex, 2, GL_FLOAT, GL_FALSE, sizeof(ShaderVertex), &vertices[0].u);
				}
				else
				{
					glDisableVertexAttribArray(kTexCoordAttributeIndex);
					glVertexAttrib2f(kTexCoordAttributeIndex, 0.0f, 0.0f);
				}

				if (use_vertex_color)
				{
					glEnableVertexAttribArray(kColorAttributeIndex);
					glVertexAttribPointer(kColorAttributeIndex, 4, GL_FLOAT, GL_FALSE, sizeof(ShaderVertex), &vertices[0].r);
				}
				else
				{
					glDisableVertexAttribArray(kColorAttributeIndex);
					glVertexAttrib4f(kColorAttributeIndex, 1.0f, 1.0f, 1.0f, 1.0f);
				}

				glDrawArrays(primitive_type, 0, static_cast<GLsizei>(vertex_count));
				glDisableVertexAttribArray(kPositionAttributeIndex);
				if (textured && m_texture_enabled)
				{
					glDisableVertexAttribArray(kTexCoordAttributeIndex);
				}
				if (use_vertex_color)
				{
					glDisableVertexAttribArray(kColorAttributeIndex);
				}
			}

			void DrawVertex3D(GLenum primitive_type, const Vertex3D* vertices, int vertex_count, bool textured, bool use_vertex_color)
			{
				if (vertices == NULL || vertex_count <= 0 || !PrepareDraw(textured, use_vertex_color))
				{
					return;
				}

				glEnableVertexAttribArray(kPositionAttributeIndex);
				glVertexAttribPointer(kPositionAttributeIndex, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), &vertices[0].x);

				if (textured && m_texture_enabled)
				{
					glEnableVertexAttribArray(kTexCoordAttributeIndex);
					glVertexAttribPointer(kTexCoordAttributeIndex, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), &vertices[0].u);
				}
				else
				{
					glDisableVertexAttribArray(kTexCoordAttributeIndex);
					glVertexAttrib2f(kTexCoordAttributeIndex, 0.0f, 0.0f);
				}

				if (use_vertex_color)
				{
					glEnableVertexAttribArray(kColorAttributeIndex);
					glVertexAttribPointer(kColorAttributeIndex, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), &vertices[0].r);
				}
				else
				{
					glDisableVertexAttribArray(kColorAttributeIndex);
					glVertexAttrib4f(kColorAttributeIndex, 1.0f, 1.0f, 1.0f, 1.0f);
				}

				glDrawArrays(primitive_type, 0, static_cast<GLsizei>(vertex_count));
				glDisableVertexAttribArray(kPositionAttributeIndex);
				if (textured && m_texture_enabled)
				{
					glDisableVertexAttribArray(kTexCoordAttributeIndex);
				}
				if (use_vertex_color)
				{
					glDisableVertexAttribArray(kColorAttributeIndex);
				}
			}

			void DrawSeparateVertexArrays(GLenum primitive_type, const float* positions, const float* tex_coords, const float* colors,
				int vertex_count, bool textured, bool use_vertex_color)
			{
				const bool textured_enabled = textured && tex_coords != NULL;
				const bool vertex_color_enabled = use_vertex_color && colors != NULL;
				if (positions == NULL || vertex_count <= 0 || !PrepareDraw(textured_enabled, vertex_color_enabled))
				{
					return;
				}

				glEnableVertexAttribArray(kPositionAttributeIndex);
				glVertexAttribPointer(kPositionAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, positions);

				if (textured_enabled)
				{
					glEnableVertexAttribArray(kTexCoordAttributeIndex);
					glVertexAttribPointer(kTexCoordAttributeIndex, 2, GL_FLOAT, GL_FALSE, 0, tex_coords);
				}
				else
				{
					glDisableVertexAttribArray(kTexCoordAttributeIndex);
					glVertexAttrib2f(kTexCoordAttributeIndex, 0.0f, 0.0f);
				}

				if (vertex_color_enabled)
				{
					glEnableVertexAttribArray(kColorAttributeIndex);
					glVertexAttribPointer(kColorAttributeIndex, 4, GL_FLOAT, GL_FALSE, 0, colors);
				}
				else
				{
					glDisableVertexAttribArray(kColorAttributeIndex);
					glVertexAttrib4f(kColorAttributeIndex, 1.0f, 1.0f, 1.0f, 1.0f);
				}

				glDrawArrays(primitive_type, 0, static_cast<GLsizei>(vertex_count));
				glDisableVertexAttribArray(kPositionAttributeIndex);
				if (textured_enabled)
				{
					glDisableVertexAttribArray(kTexCoordAttributeIndex);
				}
				if (vertex_color_enabled)
				{
					glDisableVertexAttribArray(kColorAttributeIndex);
				}
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

			Matrix4 m_projection_matrix;
			std::vector<Matrix4> m_model_view_stack;
			std::vector<SceneState> m_scene_stack;
			ShaderProgramState m_shader_program;
			bool m_initialized;
			bool m_depth_test_enabled;
			bool m_cull_face_enabled;
			bool m_depth_mask_enabled;
			bool m_texture_enabled;
			bool m_alpha_test_enabled;
			bool m_blend_enabled;
			unsigned int m_blend_src_factor;
			unsigned int m_blend_dst_factor;
			unsigned int m_depth_function;
			unsigned int m_alpha_function;
			float m_alpha_reference;
			bool m_fog_enabled;
			bool m_fog_use_range;
			float m_fog_density;
			float m_fog_start;
			float m_fog_end;
			TextureCombineMode m_texture_combine_mode;
			float m_fog_color[4];
			float m_current_color[4];
			int m_viewport_x;
			int m_viewport_y;
			int m_viewport_width;
			int m_viewport_height;
			int m_viewport_window_height;
		};

		OpenGLES2RenderBackend g_open_gles_render_backend;
	}

	bool IsOpenGLESRenderBackendAvailable()
	{
		return IsShaderPipelineAvailable();
	}

	RenderBackend* CreateOpenGLESRenderBackend()
	{
		if (!g_open_gles_render_backend.Initialize())
		{
			return NULL;
		}

		return &g_open_gles_render_backend;
	}

	void ShutdownOpenGLESRenderBackend()
	{
		g_open_gles_render_backend.Shutdown();
	}
}
