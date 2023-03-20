#pragma once

#include "Renderer.h"

namespace wv
{
	class Window;
	class Camera;

	class Application
	{
	public:
		Application();
		~Application();

		/// @brief Runs the application and initializes the renderer
		void Run();

		/// @brief Performs application teardown logic
		void Teardown() const;

	private:
		void UpdateControls(bool* keys);

	private:
		Window* m_Window = nullptr;
		Camera* m_Camera = nullptr;
		core::Unique<Renderer> m_Renderer = nullptr;

		bool m_IsMouseLocked = 0;
		bool m_IsMouseDown = false;
		bool m_FirstFrame = true;
		float m_LastX = 0.0f;
		float m_LastY = 0.0f;
	};
}
