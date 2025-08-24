#include "raylib.h"
#include "include/dialog.h"
#include "include/character.h"
#include "include/achievement.h"
#include "include/Circle.h"
int main() {
	const int screenWidth = 800;
	const int screenHeight = 600;
	int canwalk = 1;

	InitWindow(screenWidth, screenHeight, "NPC对话系统");
	InitFontSystem("C:\\Windows\\Fonts\\simhei.ttf");
	
	

	AchievementSystem achievementSys;
	achievementSys.Init();

	achievementSys.AddAchievement({"first", "踩踩背", "第一次踩背", false, ACH_COMMON});
	achievementSys.AddAchievement({"rare", "超级踩背王", "踩100+个人的背", false, ACH_RARE});
	achievementSys.AddAchievement({"zfx","同城可约","与学姐月跑",false,ACH_COMMON});
	achievementSys.Read();

	DialogSystem dialogSystem;
	dialogSystem.AddDialog(1, "ZFX学姐", "同城月跑，有钱月吗", "resource/zfx.png", 2);
	dialogSystem.AddDialog(2, "ZFX学姐", "哈哈骗你的没有头月不了", "resource/zfx.png", 3);
	dialogSystem.AddDialog(3, "general0826","没有困难的题目，只有勇敢的gengen","resource/gen.png",-1);

	// 创建系统对象
	Character player;
	CollisionSystem collisionSystem;
	CameraSystem cameraSystem;

	// 加载角色素材
	bool textureLoaded = player.LoadCharacterSheet("resource/zfx_r.png");

	// 设置角色初始位置
	player.SetPosition({screenWidth / 2.0f, screenHeight / 2.0f});

	// 设置精灵表布局（如果素材布局不同可以修改这里）
	// player.SetSpriteLayout(0, 1, 2, 3); // 默认就是这样的布局

	// 添加碰撞箱
	collisionSystem.AddCollisionBox({200, 200, 100, 50}, BLUE, true, "箱子1");
	collisionSystem.AddCollisionBox({500, 300, 80, 120}, GREEN, true, "箱子2");
	collisionSystem.AddCollisionBox({800, 150, 150, 40}, YELLOW, true, "长平台");
	collisionSystem.AddCollisionBox({300, 500, 60, 60}, ORANGE, true, "方块");
	collisionSystem.AddCollisionBox({700, 600, 120, 30}, PURPLE, true, "平台");
	collisionSystem.AddCollisionBox({400, 400, 70, 70}, GRAY, false, "可穿过");

	Vector2 worldSize = {screenWidth * 3, screenHeight * 3};
	
	Circle circle;

	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		
		if (IsKeyPressed(KEY_E) && !dialogSystem.IsActive()) {
			dialogSystem.StartDialog(1);
			achievementSys.Unlock("zfx");
			canwalk = 0;
		}

		if (dialogSystem.IsActive()) {
			dialogSystem.Update();
			canwalk = dialogSystem.HandleInput();
		}

		if (IsKeyPressed(KEY_ONE)) {
			achievementSys.Unlock("first");
		}
		if (IsKeyPressed(KEY_TWO)) {
			achievementSys.Unlock("rare");
		}
		if(IsKeyPressed(KEY_O)) {
			circle.start();
		}

		achievementSys.Update();

		float deltaTime = GetFrameTime();

		// 保存旧位置用于碰撞检测
		Vector2 oldPosition = player.GetPosition();
		Rectangle oldCollision = player.GetCollisionBox();

		// 处理输入
		if (canwalk) {
			player.HandleInput();
			player.Update(deltaTime);
		}

		// 碰撞检测
		if (collisionSystem.CheckCollision(player.GetCollisionBox())) {
			player.ResolveCollision(oldPosition, oldCollision);
		}

		// 边界检查
		player.CheckWorldBounds(worldSize);

		// 更新相机
		cameraSystem.Update(player.GetPosition());

		BeginDrawing();

		ClearBackground(SKYBLUE);

		cameraSystem.BeginMode();

		// 绘制背景网格
		for (int x = 0; x < worldSize.x; x += 50) {
			DrawLine(x, 0, x, worldSize.y, LIGHTGRAY);
		}
		for (int y = 0; y < worldSize.y; y += 50) {
			DrawLine(0, y, worldSize.x, y, LIGHTGRAY);
		}

		// 绘制碰撞箱
		collisionSystem.Draw();

		// 绘制角色
		player.Draw();

		// 调试显示碰撞箱
		if (IsKeyDown(KEY_C)) {
			player.DrawCollisionDebug();
		}

		cameraSystem.EndMode();

		// 绘制UI
		DrawTextUTF("使用WASD或方向键移动", Vector2{10, 10}, 20, 1, DARKGRAY);
		DrawTextUTF("按C键显示碰撞箱", Vector2{10, 40}, 20, 1, DARKGRAY);

		// 显示角色状态
		std::string statusText = "状态: " +
		                         CharacterUtils::StateToString(player.GetState()) + " - " +
		                         CharacterUtils::DirectionToString(player.GetDirection());
		DrawTextUTF(statusText, Vector2{10, 70}, 20, 1, DARKBLUE);

		if (!textureLoaded) {
			DrawTextUTF("无法加载角色纹理，使用替代图形", Vector2{10, 100}, 20, 1, ORANGE);
		}

		DrawFPS(screenWidth - 100, 10);

		achievementSys.Draw();

		if (dialogSystem.IsActive()) dialogSystem.Draw();

		DrawTextUTF("按 E 开始对话", {10, 10}, 20, 1, DARKGRAY);
		
		circle.out(canwalk,screenHeight,screenWidth);
		circle.photo(screenHeight,screenWidth);
		circle.in(canwalk,screenHeight,screenWidth);
		
		
		EndDrawing();
	}

	UnloadFontSystem();
	achievementSys.Save();
	player.UnloadResources();
	CloseWindow();
	return 0;
}
