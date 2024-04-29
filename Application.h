#pragma once

#include "Vector2D.h"
#include <vector>
#include <string>
#include <set>

#define VISUALIZE_SEEKING true
#define ASK_USER_FOR_INPUT false
#define GENERATE_MAZE false
#define GENERATE_CUSTOM false
#define GENERATE_FURTHEST_AWAY true

enum SlotType : uint8_t {
	Empty,
	Wall,
	User,
	Seeker,
	Target,
	FoundPath,
	Debug,
};

struct Slot {
	Slot() {}
	Slot(const Vector2D& InVec) : Position(InVec) {}
	Slot(const Vector2D& InVec, const SlotType& InType)
		: Position(InVec)
	{
		SetType(InType);
	}

	Vector2D Position{ 0 };
	SlotType Type{ SlotType::Empty };
	std::string TypeStr{ " " };

	int SeekTicks = 0;
	struct Slot* NextSeeker{ nullptr };
	struct Slot* PreviousSeeker{ nullptr };

	// Slot indices adjacent to this. 
	// contains slots in a 3x3 excluding this instance of slot.
	// also excludes walls and diagonal unreachable slots
	std::set<struct Slot*> AdjacentSlots{};

	inline bool HasAdjacentSlot(const Vector2D& In) const
	{
		for (struct Slot* AdjSlot : AdjacentSlots) 
			if (AdjSlot->Position == In) return true;
	}

	inline bool operator == (const Vector2D& In) const
	{
		return In == Position;
	}

	inline bool IsEmpty() const
	{
		return TypeStr == " ";
	}

	inline void SetType(const SlotType& InType)
	{
		if (InType == Type) return;

		Type = InType;
		switch (InType)
		{
		case SlotType::Empty: [[likely]]
			TypeStr = " ";
			break;
		case SlotType::Wall: [[likely]]
			TypeStr = "#";
			break;
		case SlotType::Seeker: [[likely]]
			TypeStr = "\033[1;34;40mO\033[0m";
			SeekTicks = 1;
			break;
		case SlotType::User: [[unlikely]]
			TypeStr = "\033[1;31;40mU\033[0m";
			break;
		case SlotType::Target: [[unlikely]]
			TypeStr = "\033[1;31;40mT\033[0m";
			break;
		case SlotType::FoundPath: [[unlikely]]
			TypeStr = "\033[1;32;40m+\033[0m";
			break;
		case SlotType::Debug: [[unlikely]]
			TypeStr = "D";
			break;
		}
	}
};

class Application
{
public:
	Application(int Size = 64);
	~Application();

private:
	std::vector<Slot*> Slots{};
	std::vector<std::vector<Slot*>> Rows{};

	Slot* UserSlot{ nullptr };
	Slot* TargetSlot{ nullptr };

	void InitializeSeeker();
	void InitializeTarget();
	void VisualizeSlots();

	void StartSeeking();
	void TickSeekers();

	void StartTraversing();
	void Traverse();

	bool bSeeking{ false };

	inline Slot* FindSlot(Slot* In) const
	{
		for (int i = 0; i < Slots.size(); i++)
			if (Slots[i]->Position == In->Position)
				return Slots[i];
		return nullptr;
	}
	inline Slot* FindWallSlotByVector(const Vector2D& In) const
	{
		for (int i = 0; i < Slots.size(); i++)
			if (Slots[i]->Position == In 
				&& Slots[i]->Type == SlotType::Wall)
					return Slots[i];
		return nullptr;
	}
};

