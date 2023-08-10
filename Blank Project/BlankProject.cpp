#include "../nclgl/window.h"
#include "Renderer.h"

/*
Press 1:	PBR with IBL model
Press 2:	PBR without IBL model
Press 3:	BlinnPhong model (Basic)
*/

int main() {
	Window w("Render success!", 1920, 1080, true);
	if (!w.HasInitialised()) {
		return -1;
	}

	Renderer renderer(w);
	if (!renderer.HasInitialised()) {
		return -1;
	}

	w.LockMouseToWindow(true);
	w.ShowOSPointer(false);
	
	


	int scene = 1;

	while (w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1)) {
			scene = 1;
		}
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2)) {
			scene = 2;
		}
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_3)) {
			scene = 3;
		}
		

		float timestep = w.GetTimer()->GetTimeDeltaSeconds();

		//Select Camera
		
		renderer.UpdateScene(timestep);
		

		//Select Scene
		if (scene == 1)
		{
			renderer.RenderScene();
			renderer.SwapBuffers();
			if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
				Shader::ReloadAllShaders();
			}
		}
		else if (scene == 2)
		{
			renderer.RenderNoIBLScene();
			renderer.SwapBuffers();
			if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
				Shader::ReloadAllShaders();
			}
		}
		else if (scene == 3)
		{
			renderer.RenderBlinnPhongScene();
			renderer.SwapBuffers();
			if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
				Shader::ReloadAllShaders();
			}
		}

	}
	return 0;
}