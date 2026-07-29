// examples/demo.ls - LuminusScript demo showcasing language features
// =====================================================================

// Variables (mutable) and constants
var playerHealth = 100
const MAX_HEALTH = 100
var frameCount = 0

// Function declaration
fun calculateDamage(base, multiplier) {
    return base * multiplier
}

// Event handler - triggered by `emit "GameStart"`
on "GameStart" -> {
    print("=== LuminusScript Demo ===")
    print("Player health: " + playerHealth)
}

// Entity lifecycle - called automatically when an entity with this script starts
fun onStart(self) {
    print("Entity " + self + " started!")
    print("Position: " + self.position)
}

// Called every frame
fun onUpdate(self, dt) {
    frameCount = frameCount + 1
    
    // Movement with arrow keys
    // KEY_LEFT = 263, KEY_RIGHT = 262, KEY_UP = 265, KEY_DOWN = 264
    if (keyDown(263)) {
        self.x = self.x - 200 * dt
    }
    if (keyDown(262)) {
        self.x = self.x + 200 * dt
    }
    if (keyDown(265)) {
        self.y = self.y - 200 * dt
    }
    if (keyDown(264)) {
        self.y = self.y + 200 * dt
    }
    
    // Emit a custom event every 60 frames
    if (frameCount % 60 == 0) {
        emit "Tick"
    }
}

// Smart contract - verifiable game rule
contract "PlayerSafety" {
    verify playerHealth > 0
    verify playerHealth <= MAX_HEALTH
}

// List operations
fun processItems(items) {
    var total = 0
    for (var item in items) {
        total = total + item
    }
    return total
}

// Conditional logic
fun checkStatus() {
    if (playerHealth > 75) {
        return "Healthy"
    } else if (playerHealth > 50) {
        return "Wounded"
    } else if (playerHealth > 0) {
        return "Critical"
    } else {
        return "Dead"
    }
}
