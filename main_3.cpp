#include "raylib.h"

int main() {
	const int screenWidth = 800;
	const int screenHeight = 600;
	int time=0;
	
	InitWindow(screenWidth, screenHeight, "NPC对话系统");
	
	Image image_ce = LoadImage("resource/ce.png");
	Image image_re = LoadImage("resource/re.png");
	Image image_tle = LoadImage("resource/tle.png");
	Image image_wa = LoadImage("resource/wa.png");
	Image image_ac = LoadImage("resource/ac.png");
	
	// 将图片转换为纹理
	Texture2D texture_ce = LoadTextureFromImage(image_ce);
	Texture2D texture_re = LoadTextureFromImage(image_re);
	Texture2D texture_tle = LoadTextureFromImage(image_tle);
	Texture2D texture_wa = LoadTextureFromImage(image_wa);
	Texture2D texture_ac = LoadTextureFromImage(image_ac);
	
	SetTargetFPS(60);
	
	while (!WindowShouldClose()) {
		
		++time;
		
		BeginDrawing();
		
		ClearBackground(BLACK);
		
		if(time<=120)
			DrawTexture(texture_ce, screenWidth/2 - texture_ce.width/2, screenHeight/2 - texture_ce.height/2, WHITE);
		if(time>120&&time<=240)
			DrawTexture(texture_re, screenWidth/2 - texture_re.width/2, screenHeight/2 - texture_re.height/2, WHITE);
		if(time>240&&time<=360)
			DrawTexture(texture_tle, screenWidth/2 - texture_tle.width/2, screenHeight/2 - texture_tle.height/2, WHITE);
		if(time>360&&time<=480)
			DrawTexture(texture_wa, screenWidth/2 - texture_wa.width/2, screenHeight/2 - texture_wa.height/2, WHITE);
		if(time>480&&time<=600)
			DrawTexture(texture_ac, screenWidth/2 - texture_ac.width/2, screenHeight/2 - texture_ac.height/2,WHITE);
		
		EndDrawing();
	}
	UnloadTexture(texture_ce);
	UnloadTexture(texture_re);
	UnloadTexture(texture_tle);
	UnloadTexture(texture_wa);
	UnloadTexture(texture_ac);
	CloseWindow();
	return 0;
}
