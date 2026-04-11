package oily.top.game.npc;

import java.util.ArrayList;
import java.util.List;

public class NPC {
    private int id;
    private String name;
    private int x;
    private int y;
    private String dialog;
    private String symbol;
    private List<NPCTask> tasks;
    private NPCDialogueType dialogueType;

    
    public enum NPCDialogueType {
        GREETING, QUEST, TRADE, TRAINING, STORY
    }
    
    public NPC(int id, String name, int x, int y) {
        this.id = id;
        this.name = name;
        this.x = x;
        this.y = y;
        this.tasks = new ArrayList<>();
        this.dialogueType = NPCDialogueType.GREETING;
        this.symbol = "👤";
    }
    
    public NPC(int id, String name, int x, int y, String dialog) {
        this(id, name, x, y);
        this.dialog = dialog;
    }
    
    // Getters
    public int getId() { return id; }
    public String getName() { return name; }
    public int getX() { return x; }
    public int getY() { return y; }
    public String getDialog() { return dialog; }
    public String getSymbol() { return symbol; }
    public List<NPCTask> getTasks() { return tasks; }
    public NPCDialogueType getDialogueType() { return dialogueType; }
    
    // Setters
    public void setDialog(String dialog) { this.dialog = dialog; }
    public void setSymbol(String symbol) { this.symbol = symbol; }
    public void setDialogueType(NPCDialogueType type) { this.dialogueType = type; }
    public void addTask(NPCTask task) { tasks.add(task); }
    public boolean hasTask(){return !tasks.isEmpty();}
}