#include "raylib.h"
#include <memory>
#include <string>
#include <iostream>
#include "1.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main() {
	// 初始化窗口
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "完整的物体碰撞系统");
	SetTargetFPS(60);
	
	// 初始化字体系统
	if (!InitFontSystem("C:\\Windows\\Fonts\\simhei.ttf")) {
		TraceLog(LOG_WARNING, "无法加载字体文件，使用默认字体");
	}
	
	// 创建物体管理系统
	GameObjectSystem gameObjects;
	CameraSystem camera;
	
	// 创建玩家角色
	auto player = std::make_shared<Character>("player");
	if (player->LoadCharacterSheet("resource/character.png")) {
		TraceLog(LOG_INFO, "角色贴图加载成功");
	} else {
		TraceLog(LOG_WARNING, "使用备用角色贴图");
	}
	player->SetPosition({400, 300});
	player->SetSpeed(150.0f);
	player->SetAnimationSpeed(0.15f);
	player->SetSpriteLayout(0, 1, 2, 3);
	gameObjects.AddObject("player", player);
	
	// 创建障碍物
	auto obstacle1 = std::make_shared<ImageObject>("resource/zfx.png", "rock1");
	obstacle1->SetPosition({200, 200});
	obstacle1->SetScale(0.8f);
	obstacle1->AddCollisionComponent({10, 10, 40, 40}, RED, true, "rock_collision");
	gameObjects.AddObject("rock1", obstacle1);
	
	auto obstacle2 = std::make_shared<ImageObject>("assets/tree.png", "tree1");
	obstacle2->SetPosition({600, 400});
	obstacle2->SetScale(1.2f);
	obstacle2->AddCollisionComponent({15, 60, 30, 20}, BLUE, true, "tree_trunk");
	gameObjects.AddObject("tree1", obstacle2);
	
	// 创建可收集物品
	auto coin = std::make_shared<ImageObject>("assets/coin.png", "coin1");
	coin->SetPosition({300, 500});
	coin->SetScale(0.5f);
	coin->AddCollisionComponent({5, 5, 20, 20}, YELLOW, false, "coin_area");
	gameObjects.AddObject("coin1", coin);
	
	// 游戏状态
	bool showDebug = true;
	bool collisionOccurred = false;
	std::string collisionInfo;
	int score = 0;
	
	// 游戏主循环
	while (!WindowShouldClose()) {
		float deltaTime = GetFrameTime();
		
		// 处理输入
		player->HandleInput();
		
		// 更新
		player->Update(deltaTime);
		
		// 检查世界边界
		player->CheckWorldBounds({SCREEN_WIDTH, SCREEN_HEIGHT});
		
		// 碰撞检测
		collisionOccurred = false;
		gameObjects.CheckAllCollisions([&](const std::string& id1, const std::string& id2) {
			collisionOccurred = true;
			collisionInfo = "碰撞: " + id1 + " ↔ " + id2;
			
			// 处理玩家碰撞
			if (id1 == "player" || id2 == "id2") {
				player->ResolveCollision();
				
				// 处理可收集物品
				if (id1.find("coin") != std::string::npos) {
					gameObjects.GetObject(id1)->SetVisible(false);
					score++;
					collisionInfo += " (收集!)";
				}
				if (id2.find("coin") != std::string::npos) {
					gameObjects.GetObject(id2)->SetVisible(false);
					score++;
					collisionInfo += " (收集!)";
				}
			}
		});
		
		if (!collisionOccurred) {
			collisionInfo = "无碰撞";
		}
		
		// 更新相机
		camera.Update(player->GetPosition());
		
		// 绘制
		BeginDrawing();
		ClearBackground(RAYWHITE);
		
		camera.BeginMode();
		gameObjects.DrawAll();
		if (showDebug) {
			gameObjects.DrawAllDebug();
		}
		camera.EndMode();
		
		// UI信息
		DrawText(TextFormat("分数: %d", score), 10, 10, 20, BLACK);
		DrawText(TextFormat("物体数量: %d", gameObjects.Count()), 10, 40, 20, BLACK);
		DrawText(TextFormat("玩家位置: (%.1f, %.1f)", 
							player->GetPosition().x, player->GetPosition().y), 10, 70, 20, BLACK);
		DrawText(collisionInfo.c_str(), 10, 100, 20, collisionOccurred ? RED : GREEN);
		
		// 操作说明
		DrawText("WASD/方向键: 移动", 10, SCREEN_HEIGHT - 120, 20, DARKGRAY);
		DrawText("F1: 切换调试显示", 10, SCREEN_HEIGHT - 90, 20, DARKGRAY);
		DrawText("R: 重置场景", 10, SCREEN_HEIGHT - 60, 20, DARKGRAY);
		DrawText("ESC: 退出", 10, SCREEN_HEIGHT - 30, 20, DARKGRAY);
		
		EndDrawing();
		
		// 额外控制
		if (IsKeyPressed(KEY_F1)) {
			showDebug = !showDebug;
			// 切换所有物体的碰撞箱显示
			for (const auto& [id, obj] : gameObjects.GetAllObjects()) {
				obj->SetCollisionVisible(showDebug);
			}
		}
		
		if (IsKeyPressed(KEY_R)) {
			// 重置场景
			player->SetPosition({400, 300});
			score = 0;
			// 重新显示所有可收集物品
			for (const auto& [id, obj] : gameObjects.GetAllObjects()) {
				if (id.find("coin") != std::string::npos) {
					obj->SetVisible(true);
				}
			}
		}
	}
	
	// 清理资源
	UnloadFontSystem();
	CloseWindow();
	
	return 0;
}
