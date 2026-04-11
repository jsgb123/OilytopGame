package oily.top.network;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.util.List;

public class Protocol {

    public static final int MSG_HEARTBEAT = 99;
    // 消息类型
    public static final int MSG_CONNECT_REQUEST = 1;
    public static final int MSG_CONNECT_RESPONSE = 2;
    public static final int MSG_PLAYER_MOVE = 3;
    public static final int MSG_PLAYER_JOIN = 4;
    public static final int MSG_PLAYER_LEAVE = 5;
    public static final int MSG_WORLD_STATE = 6;
    public static final int MSG_CHAT_MESSAGE = 7;

    public static final int MSG_MAP_DATA_REQUEST = 8;//地图请求
    // 路径请求和响应类型
    public static final int MSG_PATH_REQUEST = 9;
    public static final int MSG_PATH_RESPONSE = 10;
    public static final int MSG_LOGIN_EVENT = 11;
    // 任务系统
    public static final int MSG_ACCEPT_QUEST = 200;  //接受任务
    public static final int MSG_ACCEPT_QUEST_RESPONSE = 210;
    public static final int MSG_QUEST_PROGRESS = 201;
    public static final int MSG_QUEST_PROGRESS_RESPONSE = 211;
    public static final int MSG_COMPLETE_QUEST = 202;
    public static final int MSG_COMPLETE_QUEST_RESPONSE = 212;
    public static final int MSG_CLAIM_REWARD = 203;
    public static final int MSG_CLAIM_REWARD_RESPONSE = 213;
    public static final int MSG_QUEST_LIST = 204;
    public static final int MSG_QUEST_LIST_RESPONSE = 214;

    // NPC 交互
    public static final int MSG_TALK_TO_NPC = 220;        // 客户端请求
    public static final int MSG_NPC_DIALOG = 221;         // 服务器响应（普通对话）
    public static final int MSG_AVAILABLE_QUESTS = 222;   // 服务器响应（任务列表）

    // 通知
    public static final int MSG_QUEST_COMPLETE = 230; // 任务完成通知

    private static final ObjectMapper mapper = new ObjectMapper();

    static {
        mapper.setSerializationInclusion(JsonInclude.Include.NON_NULL);
    }

    public static class Message {

        public int type;
        public Object data;

        public Message() {
        }

        public Message(int type, Object data) {
            this.type = type;
            this.data = data;
        }
    }

    // 连接请求
    public static class ConnectRequest {

        public String playerName;

        public ConnectRequest() {
        }

        public ConnectRequest(String playerName) {
            this.playerName = playerName;
        }
    }

    // 连接响应
    public static class ConnectResponse {

        public int playerId;
        public float x;
        public float y;

        public ConnectResponse() {
        }

        public ConnectResponse(int playerId, float x, float y) {
            this.playerId = playerId;
            this.x = x;
            this.y = y;
        }
    }

    // 玩家移动
    public static class PlayerMove {

        public int playerId;
        public float x;
        public float y;
        public float direction;

        public PlayerMove() {
        }

        public PlayerMove(int playerId, float x, float y, float direction) {
            this.playerId = playerId;
            this.x = x;
            this.y = y;
            this.direction = direction;
        }
    }

    // 玩家数据
    public static class PlayerData {

        public int id;
        public String name;
        public float x;
        public float y;
        public float direction;

        public PlayerData() {
        }

        public PlayerData(int id, String name, float x, float y, float direction) {
            this.id = id;
            this.name = name;
            this.x = x;
            this.y = y;
            this.direction = direction;
        }
    }

    // 世界状态
    public static class WorldState {

        public List<PlayerData> players;

        public WorldState() {
        }

        public WorldState(List<PlayerData> players) {
            this.players = players;
        }
    }

    // 序列化
    public static String serialize(Message msg) throws JsonProcessingException {
        return mapper.writeValueAsString(msg);
    }

    // 反序列化
    public static Message deserialize(String json) throws JsonProcessingException {
        return mapper.readValue(json, Message.class);
    }

    // 创建消息
    public static Message createConnectResponse(int playerId, float x, float y) {
        return new Message(MSG_CONNECT_RESPONSE, new ConnectResponse(playerId, x, y));
    }

    public static Message createPlayerMove(int playerId, float x, float y, float direction) {
        return new Message(MSG_PLAYER_MOVE, new PlayerMove(playerId, x, y, direction));
    }

    public static Message createWorldState(List<PlayerData> players) {
        return new Message(MSG_WORLD_STATE, new WorldState(players));
    }
}
