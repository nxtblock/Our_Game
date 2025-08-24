#ifndef CHARACTER_H
#define CHARACTER_H

#include "raylib.h"
#include "include/nbsfont.h"
#include <string>
#include <vector>
#include <cmath>
#include <map>
#include <memory>
#include <functional>

// 角色方向枚举
enum class Direction {
	DOWN,
	LEFT,
	RIGHT,
	UP
};

// 动画状态枚举
enum class AnimationState {
	IDLE,
	WALKING
};

// 碰撞箱组件
struct CollisionComponent {
	Rectangle rect;
	Color debugColor;
	bool isSolid;
	std::string name;
	bool visible; // 是否显示碰撞箱
	
	CollisionComponent() 
	: rect({0, 0, 0, 0}), debugColor(RED), isSolid(true), name(""), visible(true) {}
	
	CollisionComponent(const Rectangle& r, const Color& c, bool solid, const std::string& n = "")
	: rect(r), debugColor(c), isSolid(solid), name(n), visible(true) {}
};

// 物体基类
class GameObject {
protected:
	std::string id;
	Vector2 position;
	bool visible;
	std::vector<CollisionComponent> collisionComponents;
	
public:
	GameObject(const std::string& objId = "") : id(objId), position({0, 0}), visible(true) {}
	virtual ~GameObject() = default;
	
	virtual void Update(float deltaTime) {}
	virtual void Draw() const = 0;
	virtual void DrawDebug() const {}
	
	// 碰撞检测
	virtual bool CheckCollision(const Rectangle& other) const;
	virtual bool CheckCollision(const GameObject& other) const;
	
	// 获取和设置方法
	std::string GetId() const { return id; }
	void SetId(const std::string& newId) { id = newId; }
	
	Vector2 GetPosition() const { return position; }
	virtual void SetPosition(const Vector2& newPos);
	
	bool IsVisible() const { return visible; }
	void SetVisible(bool isVisible) { visible = isVisible; }
	
	// 碰撞箱管理
	void AddCollisionComponent(const CollisionComponent& collision);
	void AddCollisionComponent(const Rectangle& rect, const Color& color, 
							   bool isSolid, const std::string& name = "");
	void ClearCollisionComponents();
	const std::vector<CollisionComponent>& GetCollisionComponents() const { return collisionComponents; }
	void SetCollisionVisible(bool visible);
	
	// 获取物体边界（用于粗略碰撞检测）
	virtual Rectangle GetBounds() const = 0;
};

// 物体管理系统
class GameObjectSystem {
private:
	std::map<std::string, std::shared_ptr<GameObject>> objects;
	
public:
	void AddObject(const std::string& id, std::shared_ptr<GameObject> object) {
		objects[id] = object;
	}
	
	bool RemoveObject(const std::string& id) {
		return objects.erase(id) > 0;
	}
	
	// 非 const 版本
	std::shared_ptr<GameObject> GetObject(const std::string& id) {
		auto it = objects.find(id);
		if (it != objects.end()) {
			return it->second;
		}
		return nullptr;
	}
	
	// const 版本
	std::shared_ptr<const GameObject> GetObject(const std::string& id) const {
		auto it = objects.find(id);
		if (it != objects.end()) {
			return it->second;
		}
		return nullptr;
	}
	
	void UpdateAll(float deltaTime) {
		for (auto& [id, obj] : objects) {
			if (obj->IsVisible()) {
				obj->Update(deltaTime);
			}
		}
	}
	
	void DrawAll() const {
		for (const auto& [id, obj] : objects) {
			if (obj->IsVisible()) {
				obj->Draw();
			}
		}
	}
	
	void DrawAllDebug() const {
		for (const auto& [id, obj] : objects) {
			if (obj->IsVisible()) {
				obj->DrawDebug();
			}
		}
	}
	
	// 碰撞检测
	bool CheckCollision(const std::string& id, const Rectangle& rect) const {
		auto obj = GetObject(id);
		if (obj) {
			return obj->CheckCollision(rect);
		}
		return false;
	}
	
	bool CheckCollision(const std::string& id1, const std::string& id2) const {
		auto obj1 = GetObject(id1);
		auto obj2 = GetObject(id2);
		if (obj1 && obj2) {
			return obj1->CheckCollision(*obj2);
		}
		return false;
	}
	
	// 遍历所有对象进行碰撞检测
	void CheckAllCollisions(std::function<void(const std::string&, const std::string&)> callback) const {
		std::vector<std::string> ids;
		for (const auto& [id, obj] : objects) {
			ids.push_back(id);
		}
		
		for (size_t i = 0; i < ids.size(); ++i) {
			for (size_t j = i + 1; j < ids.size(); ++j) {
				if (CheckCollision(ids[i], ids[j])) {
					callback(ids[i], ids[j]);
				}
			}
		}
	}
	
	void Clear() {
		objects.clear();
	}
	
	size_t Count() const {
		return objects.size();
	}
	
	const std::map<std::string, std::shared_ptr<GameObject>>& GetAllObjects() const {
		return objects;
	}
};

// 图片物体类
class ImageObject : public GameObject {
private:
	Texture2D texture;
	float scale;
	Color tint;
	Vector2 origin; // 绘制原点
	
public:
	ImageObject(const std::string& texturePath, const std::string& objId = "")
	: GameObject(objId), scale(1.0f), tint(WHITE), origin({0, 0}) {
		texture = LoadTexture(texturePath.c_str());
		if (texture.id == 0) {
			// 创建备用纹理
			Image fallbackImage = GenImageColor(64, 64, BLUE);
			texture = LoadTextureFromImage(fallbackImage);
			UnloadImage(fallbackImage);
		}
		
		// 自动添加基于纹理的碰撞箱
		if (texture.id != 0) {
			AddCollisionComponent(
								  {0, 0, (float)texture.width, (float)texture.height},
								  GREEN, true, "texture_bounds"
								  );
		}
	}
	
	~ImageObject() {
		if (texture.id != 0) {
			UnloadTexture(texture);
		}
	}
	
	void Draw() const override {
		if (texture.id != 0 && visible) {
			Vector2 drawPos = {
				position.x - origin.x * scale,
				position.y - origin.y * scale
			};
			DrawTextureEx(texture, drawPos, 0.0f, scale, tint);
		}
	}
	
	void DrawDebug() const override {
		if (!visible) return;
		
		// 绘制碰撞箱
		for (const auto& collision : collisionComponents) {
			if (collision.visible) {
				Rectangle worldRect = collision.rect;
				worldRect.x += position.x;
				worldRect.y += position.y;
				
				if (collision.isSolid) {
					DrawRectangleRec(worldRect, Fade(collision.debugColor, 0.5f));
					DrawRectangleLinesEx(worldRect, 2.0f, collision.debugColor);
				} else {
					DrawRectangleRec(worldRect, Fade(collision.debugColor, 0.3f));
					DrawRectangleLinesEx(worldRect, 1.0f, collision.debugColor);
				}
				
				// 绘制碰撞箱名称
				if (!collision.name.empty()) {
					DrawTextUTF(collision.name, Vector2{worldRect.x + 5, worldRect.y + 5}, 10, 1, BLACK);
				}
			}
		}
	}
	
	void SetPosition(const Vector2& newPos) override {
		GameObject::SetPosition(newPos);
		// 更新碰撞箱位置
		UpdateCollisionComponents();
	}
	
	// 获取和设置方法
	Texture2D GetTexture() const { return texture; }
	void SetTexture(Texture2D newTexture);
	
	float GetScale() const { return scale; }
	void SetScale(float newScale);
	
	Color GetTint() const { return tint; }
	void SetTint(Color newTint) { tint = newTint; }
	
	Vector2 GetOrigin() const { return origin; }
	void SetOrigin(const Vector2& newOrigin) { origin = newOrigin; }
	
	Rectangle GetBounds() const override {
		if (texture.id == 0) return {position.x, position.y, 0, 0};
		
		return {
			position.x - origin.x * scale,
			position.y - origin.y * scale,
			texture.width * scale,
			texture.height * scale
		};
	}
	
private:
	void UpdateCollisionComponents();
};

// 角色类
class Character : public GameObject {
private:
	Texture2D characterSheet;
	float speed;
	Vector2 oldPosition; // 用于碰撞解决
	
	// 动画相关变量
	Direction currentDirection;
	AnimationState currentState;
	int currentFrame;
	float animationTimer;
	float animationSpeed;
	int framesPerDirection;
	int spriteWidth;
	int spriteHeight;
	
	// 精灵表布局
	int downRow;
	int leftRow;
	int rightRow;
	int upRow;
	
public:
	Character(const std::string& objId = "");
	~Character();
	
	bool LoadCharacterSheet(const std::string& texturePath);
	void UnloadResources();
	
	void Update(float deltaTime) override;
	void Draw() const override;
	void DrawDebug() const override;
	
	// 输入处理
	void HandleInput();
	
	// 碰撞解决
	void ResolveCollision();
	
	// 边界检查
	void CheckWorldBounds(const Vector2& worldSize);
	
	// 获取方法
	Direction GetDirection() const {
		return currentDirection;
	}
	AnimationState GetState() const {
		return currentState;
	}
	float GetSpeed() const { return speed; }
	
	// 设置方法
	void SetSpeed(float newSpeed) {
		speed = newSpeed;
	}
	void SetAnimationSpeed(float newSpeed) {
		animationSpeed = newSpeed;
	}
	void SetSpriteLayout(int down, int left, int right, int up);
	
	Rectangle GetBounds() const override;
	
private:
	Rectangle GetCurrentSpriteRect() const;
	void UpdateAnimation(float deltaTime);
	void UpdateCollisionComponents();
};

// 相机系统
class CameraSystem {
private:
	Camera2D camera;
	Vector2 targetOffset;
	
public:
	CameraSystem();
	void Update(const Vector2& targetPosition);
	void BeginMode() const;
	void EndMode() const;
	
	void SetOffset(const Vector2& offset) {
		targetOffset = offset;
	}
	Camera2D GetCamera() const {
		return camera;
	}
	
	void SetZoom(float zoom) {
		camera.zoom = zoom;
	}
	float GetZoom() const {
		return camera.zoom;
	}
};

// ==================== GameObject 实现 ====================

bool GameObject::CheckCollision(const Rectangle& other) const {
	for (const auto& collision : collisionComponents) {
		if (collision.isSolid) {
			Rectangle worldRect = collision.rect;
			worldRect.x += position.x;
			worldRect.y += position.y;
			
			if (CheckCollisionRecs(worldRect, other)) {
				return true;
			}
		}
	}
	return false;
}

bool GameObject::CheckCollision(const GameObject& other) const {
	for (const auto& myCollision : collisionComponents) {
		if (myCollision.isSolid) {
			Rectangle myWorldRect = myCollision.rect;
			myWorldRect.x += position.x;
			myWorldRect.y += position.y;
			
			for (const auto& otherCollision : other.collisionComponents) {
				if (otherCollision.isSolid) {
					Rectangle otherWorldRect = otherCollision.rect;
					otherWorldRect.x += other.position.x;
					otherWorldRect.y += other.position.y;
					
					if (CheckCollisionRecs(myWorldRect, otherWorldRect)) {
						return true;
					}
				}
			}
		}
	}
	return false;
}

void GameObject::SetPosition(const Vector2& newPos) {
	position = newPos;
}

void GameObject::AddCollisionComponent(const CollisionComponent& collision) {
	collisionComponents.push_back(collision);
}

void GameObject::AddCollisionComponent(const Rectangle& rect, const Color& color, 
									   bool isSolid, const std::string& name) {
	collisionComponents.emplace_back(rect, color, isSolid, name);
}

void GameObject::ClearCollisionComponents() {
	collisionComponents.clear();
}

void GameObject::SetCollisionVisible(bool visible) {
	for (auto& collision : collisionComponents) {
		collision.visible = visible;
	}
}

// ==================== ImageObject 实现 ====================

void ImageObject::SetTexture(Texture2D newTexture) {
	if (texture.id != 0) {
		UnloadTexture(texture);
	}
	texture = newTexture;
	UpdateCollisionComponents();
}

void ImageObject::SetScale(float newScale) {
	scale = newScale;
	UpdateCollisionComponents();
}

void ImageObject::UpdateCollisionComponents() {
	// 更新基于纹理的碰撞箱
	for (auto& collision : collisionComponents) {
		if (collision.name == "texture_bounds" && texture.id != 0) {
			collision.rect.width = texture.width * scale;
			collision.rect.height = texture.height * scale;
		}
	}
}

// ==================== Character 实现 ====================

Character::Character(const std::string& objId)
: GameObject(objId), speed(200.0f), currentDirection(Direction::DOWN),
currentState(AnimationState::IDLE), currentFrame(0), animationTimer(0.0f),
animationSpeed(0.1f), framesPerDirection(4), spriteWidth(0), spriteHeight(0),
downRow(0), leftRow(1), rightRow(2), upRow(3) {
	characterSheet = {0};
	oldPosition = {0, 0};
}

Character::~Character() {
	UnloadResources();
}

bool Character::LoadCharacterSheet(const std::string& texturePath) {
	characterSheet = LoadTexture(texturePath.c_str());
	if (characterSheet.id == 0) {
		Image fallbackImage = GenImageColor(64, 64, RED);
		characterSheet = LoadTextureFromImage(fallbackImage);
		UnloadImage(fallbackImage);
		return false;
	}
	
	spriteWidth = characterSheet.width / 4;
	spriteHeight = characterSheet.height / 4;
	
	// 设置角色碰撞箱（位于脚部）
	float collisionWidth = spriteWidth * 0.5f;
	float collisionHeight = spriteHeight * 0.25f;
	
	AddCollisionComponent(
						  {-collisionWidth / 2.0f, spriteHeight / 2.0f - collisionHeight, collisionWidth, collisionHeight},
						  RED, true, "character_feet"
						  );
	
	return true;
}

void Character::UnloadResources() {
	if (characterSheet.id != 0) {
		UnloadTexture(characterSheet);
	}
}

void Character::HandleInput() {
	oldPosition = position; // 保存旧位置用于碰撞解决
	
	Vector2 movement = {0, 0};
	bool isMoving = false;
	
	if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
		movement.x += 1;
		currentDirection = Direction::RIGHT;
		isMoving = true;
	}
	if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
		movement.x -= 1;
		currentDirection = Direction::LEFT;
		isMoving = true;
	}
	if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
		movement.y -= 1;
		currentDirection = Direction::UP;
		isMoving = true;
	}
	if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
		movement.y += 1;
		currentDirection = Direction::DOWN;
		isMoving = true;
	}
	
	currentState = isMoving ? AnimationState::WALKING : AnimationState::IDLE;
	
	if (isMoving) {
		if (movement.x != 0 && movement.y != 0) {
			movement.x *= 0.7071f;
			movement.y *= 0.7071f;
		}
		position.x += movement.x * speed * GetFrameTime();
		position.y += movement.y * speed * GetFrameTime();
	}
}

void Character::Update(float deltaTime) {
	UpdateAnimation(deltaTime);
}

void Character::ResolveCollision() {
	position = oldPosition; // 回到碰撞前的位置
}

void Character::UpdateAnimation(float deltaTime) {
	if (currentState == AnimationState::WALKING) {
		animationTimer += deltaTime;
		if (animationTimer >= animationSpeed) {
			animationTimer = 0.0f;
			currentFrame = (currentFrame + 1) % framesPerDirection;
		}
	} else {
		currentFrame = 0;
		animationTimer = 0.0f;
	}
}

Rectangle Character::GetCurrentSpriteRect() const {
	int row = 0;
	switch (currentDirection) {
		case Direction::DOWN: row = downRow; break;
		case Direction::LEFT: row = leftRow; break;
		case Direction::RIGHT: row = rightRow; break;
		case Direction::UP: row = upRow; break;
	}
	return {
		(float)(currentFrame * spriteWidth),
		(float)(row * spriteHeight),
		(float)spriteWidth,
		(float)spriteHeight
	};
}

void Character::Draw() const {
	if (characterSheet.id == 0 || !visible) return;
	
	Rectangle sourceRect = GetCurrentSpriteRect();
	DrawTexturePro(
				   characterSheet,
				   sourceRect,
				   { position.x, position.y, (float)spriteWidth, (float)spriteHeight },
				   { spriteWidth / 2.0f, spriteHeight / 2.0f },
				   0.0f,
				   WHITE
				   );
}

void Character::DrawDebug() const {
	if (!visible) return;
	
	// 绘制碰撞箱
	for (const auto& collision : collisionComponents) {
		if (collision.visible) {
			Rectangle worldRect = collision.rect;
			worldRect.x += position.x;
			worldRect.y += position.y;
			
			if (collision.isSolid) {
				DrawRectangleRec(worldRect, Fade(collision.debugColor, 0.5f));
				DrawRectangleLinesEx(worldRect, 2.0f, collision.debugColor);
			} else {
				DrawRectangleRec(worldRect, Fade(collision.debugColor, 0.3f));
				DrawRectangleLinesEx(worldRect, 1.0f, collision.debugColor);
			}
			
			// 绘制碰撞箱名称
			if (!collision.name.empty()) {
				DrawTextUTF(collision.name, Vector2{worldRect.x + 5, worldRect.y + 5}, 10, 1, BLACK);
			}
		}
	}
}

void Character::CheckWorldBounds(const Vector2& worldSize) {
	Rectangle bounds = GetBounds();
	if (position.x < bounds.width / 2.0f) {
		position.x = bounds.width / 2.0f;
	}
	if (position.y < bounds.height / 2.0f) {
		position.y = bounds.height / 2.0f;
	}
	if (position.x > worldSize.x - bounds.width / 2.0f) {
		position.x = worldSize.x - bounds.width / 2.0f;
	}
	if (position.y > worldSize.y - bounds.height / 2.0f) {
		position.y = worldSize.y - bounds.height / 2.0f;
	}
}

void Character::SetSpriteLayout(int down, int left, int right, int up) {
	downRow = down;
	leftRow = left;
	rightRow = right;
	upRow = up;
}

Rectangle Character::GetBounds() const {
	return {
	position.x - spriteWidth / 2.0f,
	position.y - spriteHeight / 2.0f,
	(float)spriteWidth,
	(float)spriteHeight
};
}

void Character::UpdateCollisionComponents() {
	// 如果需要更新碰撞箱，可以在这里实现
}

// ==================== CameraSystem 实现 ====================

CameraSystem::CameraSystem() {
	camera = {0};
	camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;
	targetOffset = {0, 0};
}

void CameraSystem::Update(const Vector2& targetPosition) {
	camera.target = targetPosition;
}

void CameraSystem::BeginMode() const {
	BeginMode2D(camera);
}

void CameraSystem::EndMode() const {
	EndMode2D();
}

#endif // CHARACTER_H
