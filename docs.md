


# `bot_t *make_bot(const char *token, int intents);`

This is the entry point for you bot. You can set a Environment variable and get the token using `getenv()` and pass it as the fast parameter to the function.

The second parameter are various events the discord api can send you. You can bitwise OR these various intents.

## Example
```c
    bot_t *my_bot = make_bot(getenv("TOKEN"), GUILD_MESSAGES | GUILD_MESSAGE_REACTIONS)
```

# `void run(bot_t *bot);`

This will "run" your bot and connect it to the discord gateway.

This function will block.

## Example
```c
    bot_t *my_bot = make_bot(getenv("TOKEN"), GUILD_MESSAGES | GUILD_MESSAGE_REACTIONS)
    run(my_bot);
```


