#include <glm/glm.hpp>

/*
Calculates a world direction vector from given mouseposition.
Algorithm source from http://antongerdelan.net/opengl/raycasting.html
*/
void ScreenPosToWorldRay(
	int mouseX, int mouseY,             // Mouse position, in pixels, in window coordinates
	int viewportWidth, int viewportHeight,  // Viewport size, in pixels
	glm::mat4 ViewMatrix,               // Camera position and orientation
	glm::mat4 ProjectionMatrix,         // Camera parameters (ratio, field of view, near and far planes)
	glm::vec3& out_direction            // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
) {

	// The ray in Normalized Device Coordinates
	glm::vec4 lRay_NDC(
		2.0 * mouseX / (float)viewportWidth - 1.0f, //convert to range [-1;1]
		1.0f - 2.0f * mouseY / viewportHeight, //convert to range [-1;1] and reverse direction (window y increases top to bottom, opengl y increases bottom to top
		-1.0, //z = -1 is usually the forward direction
		1.0f //w not important
	);


	// The Projection matrix goes from Camera Space to NDC.
	// So inverse(ProjectionMatrix) goes from NDC to Camera Space.
	glm::mat4 InverseProjectionMatrix = glm::inverse(ProjectionMatrix);

	// The View Matrix goes from World Space to Camera Space.
	// So inverse(ViewMatrix) goes from Camera Space to World Space.
	glm::mat4 InverseViewMatrix = glm::inverse(ViewMatrix);

	glm::vec4 lRay_camera = glm::vec4(InverseProjectionMatrix * lRay_NDC);
	lRay_camera.z = -1;
	lRay_camera.w = 0; //a vector, not a point
	glm::vec4 lRay_world = InverseViewMatrix * lRay_camera;

	out_direction = glm::normalize(lRay_world);
}

