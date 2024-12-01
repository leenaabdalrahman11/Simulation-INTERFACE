class MazeSimulator:
    def __init__(self, maze):
        self.maze = maze  # 2D list representing the maze
        self.rows = len(maze)
        self.cols = len(maze[0])

    def print_maze(self):
        """Print the current state of the maze."""
        for row in self.maze:
            print(' '.join(map(str, row)))
        print()

    def is_valid(self, x, y, target_value):
        """Check if a cell is within bounds and matches the target value."""
        return 0 <= x < self.rows and 0 <= y < self.cols and self.maze[x][y] == target_value

    def flood_fill(self, x, y, replacement_value):
        """Perform flood fill starting from (x, y)."""
        target_value = self.maze[x][y]  # Original value to replace
        if target_value == replacement_value:  # Prevent infinite loop
            return

        stack = [(x, y)]  # Stack for iterative flood fill
        while stack:
            cx, cy = stack.pop()
            if self.is_valid(cx, cy, target_value):
                self.maze[cx][cy] = replacement_value
                # Add neighbors to the stack
                stack.extend([(cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)])

# Example Maze (8x8)
maze = [
    [0, 0, 1, 1, 0, 0, 0, 1],
    [1, 0, 0, 1, 1, 1, 0, 0],
    [1, 1, 0, 0, 0, 1, 1, 0],
    [0, 0, 0, 1, 0, 0, 0, 1],
    [1, 1, 0, 1, 0, 1, 1, 1],
    [0, 0, 0, 0, 0, 1, 0, 0],
    [1, 1, 1, 1, 0, 1, 1, 0],
    [0, 0, 0, 1, 0, 0, 0, 0],
]

# Initialize the simulator
sim = MazeSimulator(maze)

# Print the original maze
print("Original Maze:")
sim.print_maze()

# Apply flood fill starting at (0, 0) with replacement value 2
sim.flood_fill(0, 0, 2)

# Print the maze after flood fill
print("Maze After Flood Fill:")
sim.print_maze()