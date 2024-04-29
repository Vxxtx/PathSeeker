#include "Application.h"
#include <random>
#include <iostream>
#include <stdlib.h>
#include <thread>

Application::Application(int Size)
{
	srand(time(0));

#if GENERATE_MAZE
	Size = 24;
	for (int i = 0; i < Size; i++) {
		Rows.emplace_back(std::vector<Slot*>());
		
		for (int j = 0; j < Size * 2; j++)
		{
			Slot* NewSlot = new Slot(
				Vector2D(j, i), i == 0 || j == 0 || i == Size - 1 || j == Size * 2 - 1  // edges
					|| i % 2 == 0 
						&& (j != Size * 2 - 2 && j != 1 || j == Size * 2 - 2 && i % 4 == 0 || j == 1 && i % 4 == 2) // flip-flop between sides where wall is open
					? Wall
					: Empty);

			Slots.emplace_back(NewSlot);
			Rows[i].emplace_back(NewSlot);
		}
	}
#elif GENERATE_CUSTOM
	// Meant to be used with 24 size
	for (int i = 0; i < Size; i++)
	{
		Rows.emplace_back(std::vector<Slot*>());
		for (int j = 0; j < Size * 2; j++)
		{
			Slot* NewSlot = new Slot(
				Vector2D(j, i),
				(i == 0 || j == 0 || i == Size - 1 || j == Size * 2 - 1)
					? Wall
					: Empty);
			if (i > 4 && i < 13 && j > 30 && j < 35)
				NewSlot->SetType(SlotType::Wall);
			if (j < 11 || (i > 15 && i < 17 && !(j > 24 && j < 30)) || (i > 16 && j > 30))
				NewSlot->SetType(SlotType::Wall);
			if (i == 12 && j < 25 || (j == 24 && i == 13) || (j == 24 && i == 15))
				NewSlot->SetType(SlotType::Wall);

			Slots.emplace_back(NewSlot);
			Rows[i].emplace_back(NewSlot);
		}
	}

	Slots[767]->SetType(SlotType::Target);
	TargetSlot = Slots[766];
#else
	for (int i = 0; i < Size; i++)
	{
		Rows.emplace_back(std::vector<Slot*>());
		for (int j = 0; j < Size * 2; j++)
		{
			Slot* NewSlot = new Slot(
				Vector2D(j, i),
				(rand() % 4 == 0 || i == 0 || j == 0 || i == Size - 1 || j == Size * 2 - 1)
					? Wall
					: Empty);

			Slots.emplace_back(NewSlot);
			Rows[i].emplace_back(NewSlot);
		}
	}
#endif

	for (int i = 0; i < Slots.size(); i++) {
		Slot* ASlot = Slots[i];
		for (Slot* BSlot : Slots) {
			if (BSlot->Type != SlotType::Wall &&
				BSlot->Position.IsAdjacent(ASlot->Position))
			{
				Vector2D DiagonalPos;
				bool bIsDiagonal = BSlot->Position.IsDiagonalTo(ASlot->Position, DiagonalPos);

				if (bIsDiagonal) {
					Slot* PossibleBlocker = nullptr;

					PossibleBlocker = FindWallSlotByVector(Vector2D(ASlot->Position + Vector2D(DiagonalPos.X, 0)));
					if (PossibleBlocker)
						PossibleBlocker = FindWallSlotByVector(Vector2D(ASlot->Position + Vector2D(0, DiagonalPos.Y)));

					if (!PossibleBlocker)
						ASlot->AdjacentSlots.insert(BSlot);
				}
				else 
					ASlot->AdjacentSlots.insert(BSlot);
			}
		}
	}

	InitializeSeeker();
#if !GENERATE_CUSTOM
	InitializeTarget();
#endif
	VisualizeSlots();
}

Application::~Application()
{
	for (Slot* Slot : Slots)
		delete Slot;
}

void Application::InitializeSeeker()
{
	for (Slot* Slot : Slots)
	{
		if (Slot->IsEmpty())
		{
			Slot->SetType(SlotType::User);
			UserSlot = Slot;
			break;
		}
	}
}

void Application::InitializeTarget()
{
#if GENERATE_FURTHEST_AWAY
	for (int i = Slots.size() - 1; i > 0; i--)
	{
		if (Slots[i]->IsEmpty())
		{
			Slots[i]->SetType(SlotType::Target);
			TargetSlot = Slots[i];
			break;
		}
	}
#else 
	int RandomPos = (rand() % (Slots.size() + 1));
	Slots[RandomPos]->SetType(SlotType::Target);
	TargetSlot = Slots[RandomPos];
#endif
}

void Application::VisualizeSlots()
{
	std::string ColumnRowString = "";
	for (int i = 0; i < Rows.size(); i++) {
		ColumnRowString += "\n";
		for (int j = 0; j < Rows[i].size(); j++) {
			if (Rows[i][j]->Type == SlotType::User)
			{
				ColumnRowString += Rows[i][j]->TypeStr;
			}
			else 
				ColumnRowString += Rows[i][j]->TypeStr;
		}
	}
	system("cls");
	std::cout << ColumnRowString;

	if (bSeeking) return;

#if ASK_USER_FOR_INPUT
	std::string Result;
	std::cout << "\n\nStart seeking? Y/N: ";
	std::cin >> Result;

	if (Result == "Y" || Result == "y")
#endif
		StartSeeking();
}

void Application::StartSeeking()
{
	bSeeking = true;
	if (UserSlot && TargetSlot) {
		for (Slot* AdjSlot : UserSlot->AdjacentSlots) {
			AdjSlot->SetType(SlotType::Seeker);
		}
	}
	TickSeekers();
}

void Application::TickSeekers()
{
#if VISUALIZE_SEEKING
	VisualizeSlots();
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
#endif

	std::vector<int> OldSeekerIndices = {};

	for (int i = 0; i < Slots.size(); i++)
	{
		if (Slots[i]->Type == SlotType::Seeker)
			OldSeekerIndices.emplace_back(i);
	}

	Slot* FirstReacher = nullptr;

	for (const int& Index : OldSeekerIndices) {
		for (Slot* Adj : Slots[Index]->AdjacentSlots)
		{
			if (Adj->Type != SlotType::Seeker
				&& Adj->SeekTicks == 0
				&& Adj->Type != SlotType::User)
			{
				if (Adj->Type == SlotType::Target && Adj->SeekTicks == 0) {
					FirstReacher = Slots[Index];
					FirstReacher->NextSeeker = Adj;
					break;
				}
				else
				{
					Adj->SetType(SlotType::Seeker);
					Adj->PreviousSeeker = Slots[Index];
				}
			}
		}
	}

	if (OldSeekerIndices.empty())
	{
		std::cout << "\nPath not found" << std::endl;
		return;
	}

	for (const int& Index : OldSeekerIndices) {
		Slots[Index]->SetType(SlotType::Empty);
	}

	if (!FirstReacher)
		TickSeekers();
	else
	{
		for (Slot* Slot : Slots)
			if (Slot->Type == SlotType::Seeker)
				Slot->SetType(SlotType::Empty);
		if (FirstReacher->PreviousSeeker)
		{
			FirstReacher->SetType(SlotType::FoundPath);
			FirstReacher->PreviousSeeker->NextSeeker = FirstReacher;
			Slot* Next = FirstReacher->PreviousSeeker;
			Slot* First = nullptr;
			while (Next)
			{
				Next->SetType(SlotType::FoundPath);

				if (Next->PreviousSeeker == nullptr)
					First = Next;
				else
					Next->PreviousSeeker->NextSeeker = Next;
				Next = Next->PreviousSeeker;
			}
			VisualizeSlots();

			UserSlot->NextSeeker = First;

#if ASK_USER_FOR_INPUT
			std::string Result;
			std::cout << "\n\nPath found, start traversing? Y/N: ";
			std::cin >> Result;

			if (Result == "Y" || Result == "y")
#endif
				StartTraversing();
		}
	}
}

void Application::StartTraversing()
{
	Traverse();
}

void Application::Traverse()
{
	VisualizeSlots();
	if (UserSlot->NextSeeker)
	{
		UserSlot->SetType(SlotType::Empty);
		UserSlot = UserSlot->NextSeeker;
		UserSlot->SetType(SlotType::User);

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		Traverse();
	}
	else
	{
		std::cout << "\nTarget reached!" << std::endl;
	}
}
