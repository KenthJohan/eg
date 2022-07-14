while(running)
{
	SDL_GL_MakeCurrent(windows[0], contexts[0]);
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(vao[0]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	SDL_GL_SwapWindow(windows[0]);

	SDL_GL_MakeCurrent(windows[1], contexts[1]);
	glClearColor(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(vao[0]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	SDL_GL_SwapWindow(windows[1]);
}


ECS_SYSTEM(world, System_Windows, EcsOnUpdate, [in] SdlWindow);
ECS_SYSTEM(world, System_Draw, EcsOnUpdate, [in] Shapes);
