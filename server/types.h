enum change_type {
    CREATE, MODIFY, DELETE
};

struct message {
    enum change_type type;
    char file_path[256];
    char content[256]; // TODO
};