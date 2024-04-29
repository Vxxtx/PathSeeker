#pragma once

struct Vector2D {
	Vector2D() {}
	Vector2D(const int& InXY)
		: X(InXY), Y(InXY) 
	{}
	Vector2D(const int& InX, const int InY)
		: X(InX), Y(InY)
	{}
	Vector2D(const struct Vector2D& In)
		: X(In.X), Y(In.Y)
	{}

	int X{ 0 };
	int Y{ 0 };

	inline bool operator == (const Vector2D& In) const
	{
		return In.X == X && In.Y == Y;
	}

	inline Vector2D operator + (const Vector2D& In)
	{
		return Vector2D(X + In.X, Y + In.Y);
	}

	inline Vector2D operator - (const Vector2D& In)
	{
		return Vector2D(X - In.X, Y - In.Y);
	}

	inline bool IsAdjacent(const Vector2D& In) const 
	{
		if (*this == In) return false;

		return 
			(In.X + 1 == X || In.X == X || In.X - 1 == X)
			&& (In.Y + 1 == Y || In.Y == Y || In.Y - 1 == Y);
	}

	inline bool IsDiagonalTo(const Vector2D& In, Vector2D& OutDiagonalPosition) const
	{
		if (In.X + 1 == X && In.Y + 1 == Y)
		{
			OutDiagonalPosition = Vector2D(X, Y) - In;
			return true;
		}
		if (In.X - 1 == X && In.Y - 1 == Y)
		{
			OutDiagonalPosition = Vector2D(X, Y) - In;
			return true;
		}
		if (In.X + 1 == X && In.Y - 1 == Y)
		{
			OutDiagonalPosition = Vector2D(X, Y) - In;
			return true;
		}
		if (In.X - 1 == X && In.Y + 1 == Y)
		{
			OutDiagonalPosition = Vector2D(X, Y) - In;
			return true;
		}
		return false;
	}
};