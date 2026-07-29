# Luminus Script demo
# Move the player with arrow keys, jump with SPACE

when_start {
    say "Luminus Engine Started"
}

when_update {
    if key_held LEFT  { player.x -= 5 }
    if key_held RIGHT { player.x += 5 }
    if key_held UP    { player.y -= 5 }
    if key_held DOWN  { player.y += 5 }
    if key_hit SPACE  { player.y -= 20 }
    if key_held ESC   { exit }
}
