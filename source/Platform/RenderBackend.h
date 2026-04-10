#pragma once

namespace platform
{
	enum RenderBackendType
	{
		RenderBackendType_LegacyOpenGL = 0,
		RenderBackendType_OpenGLES2 = 1,
	};

	enum TextureCombineMode
	{
		TextureCombineMode_Modulate = 0,
		TextureCombineMode_Add = 1,
	};

	struct QuadVertex2D
	{
		float x;
		float y;
		float u;
		float v;
		float r;
		float g;
		float b;
		float a;
	};

	struct Vertex3D
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

	class RenderBackend
	{
	public:
		virtual ~RenderBackend() {}

		virtual void SetViewport(int x, int y, int width, int height, int window_height) = 0;
		virtual void SetClearColor(float r, float g, float b, float a) = 0;
		virtual void Clear(unsigned int clear_mask) = 0;
		virtual void Flush() = 0;
		virtual bool PresentCurrentFrame() = 0;
		virtual void PushPerspectiveScene(int viewport_x, int viewport_y, int viewport_width, int viewport_height, int window_height,
			float fov, float near_plane, float far_plane, const float camera_position[3], const float camera_angle[3],
			bool camera_top_view, bool fog_enabled, float fog_density, const float fog_color[4]) = 0;
		virtual void PushOrthoScene(int window_width, int window_height) = 0;
		virtual void PushOrthoSceneViewport(int viewport_x, int viewport_y, int viewport_width, int viewport_height, int window_height,
			float ortho_width, float ortho_height) = 0;
		virtual void PopScene() = 0;

		virtual void PushModelView() = 0;
		virtual void PushIdentityModelView() = 0;
		virtual void PopModelView() = 0;
		virtual void LoadIdentityModelView() = 0;
		virtual void RotateModelView(float angle, float x, float y, float z) = 0;
		virtual void TranslateModelView(float x, float y, float z) = 0;
		virtual void CopyModelViewMatrix(float matrix[16]) const = 0;

		virtual void SetDepthTestEnabled(bool enabled) = 0;
		virtual void SetCullFaceEnabled(bool enabled) = 0;
		virtual void SetDepthMaskEnabled(bool enabled) = 0;
		virtual void SetTextureEnabled(bool enabled) = 0;
		virtual void SetAlphaTestEnabled(bool enabled) = 0;
		virtual void SetBlendState(bool enabled, unsigned int src_factor, unsigned int dst_factor) = 0;
		virtual void SetDepthFunction(unsigned int func) = 0;
		virtual void SetAlphaFunction(unsigned int func, float ref) = 0;
		virtual void SetFogEnabled(bool enabled) = 0;
		virtual void ConfigureLinearFog(float density, const float color[4]) = 0;
		virtual void ConfigureLinearFogRange(float start_distance, float end_distance, const float color[4]) = 0;
		virtual void SetTextureCombineMode(TextureCombineMode mode) = 0;
		virtual void SetCurrentColor(float r, float g, float b, float a) = 0;

		virtual void DrawQuad2D(const QuadVertex2D vertices[4], bool textured) = 0;
		virtual void DrawTriangleFan2D(const QuadVertex2D* vertices, int vertex_count, bool textured) = 0;
		virtual void DrawQuad3D(const Vertex3D vertices[4], bool textured, bool use_vertex_color) = 0;
		virtual void DrawTriangleList3D(const Vertex3D* vertices, int vertex_count, bool textured, bool use_vertex_color) = 0;
		virtual void DrawLineList3D(const Vertex3D* vertices, int vertex_count, bool use_vertex_color) = 0;
		virtual void DrawPrimitiveArray3D(unsigned int primitive_type, const float* positions, const float* tex_coords, const float* colors,
			int vertex_count, bool textured, bool use_vertex_color) = 0;
	};

	void InitializeRenderBackend();
	void ShutdownRenderBackend();
	RenderBackend& GetRenderBackend();
	void SetPreferredRenderBackendType(RenderBackendType type);
	RenderBackendType GetPreferredRenderBackendType();
	RenderBackendType GetActiveRenderBackendType();
	bool IsRenderBackendTypeAvailable(RenderBackendType type);
	const char* GetRenderBackendTypeName(RenderBackendType type);
}
