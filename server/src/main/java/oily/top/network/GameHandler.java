package oily.top.network;

import com.fasterxml.jackson.core.JsonProcessingException;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.SimpleChannelInboundHandler;
import io.netty.handler.codec.http.websocketx.TextWebSocketFrame;
import io.netty.handler.codec.http.websocketx.WebSocketFrame;
import java.util.ArrayList;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import oily.top.game.Player;
import oily.top.game.World;
import oily.top.game.map.MapData;
import oily.top.game.map.PathFinder;
import oily.top.game.npc.NPC;
import oily.top.game.portal.Portal;
import oily.top.game.quest.PlayerQuest;
import oily.top.game.quest.Quest;
import oily.top.game.quest.Quest.QuestStatus;
import oily.top.game.quest.Quest.QuestType;
import oily.top.game.quest.QuestManager;
import oily.top.game.quest.QuestTemplate;

public class GameHandler extends SimpleChannelInboundHandler<WebSocketFrame> {

    private static final Logger logger = LoggerFactory.getLogger(GameHandler.class);

    // 会话管理
    private static final Map<String, Integer> playerSessions = new ConcurrentHashMap<>();
    private static final Map<Integer, String> sessionByPlayer = new ConcurrentHashMap<>();
    private static final Map<String, ChannelHandlerContext> sessionContexts = new ConcurrentHashMap<>();

    private final World world = World.getInstance();

    @Override
    protected void channelRead0(ChannelHandlerContext ctx, WebSocketFrame frame) throws Exception {
        if (frame instanceof TextWebSocketFrame) {
            String request = ((TextWebSocketFrame) frame).text();
            handleMessage(ctx, request);
        } else {
            logger.warn("不支持的WebSocket帧类型: {}", frame.getClass().getName());
        }
    }

    @SuppressWarnings("unchecked")
    private void handleMessage(ChannelHandlerContext ctx, String message) {
        try {
            Protocol.Message msg = Protocol.deserialize(message);
// 获取会话ID
            String sessionId = ctx.channel().id().asShortText();
            switch (msg.type) {
                case Protocol.MSG_LOGIN_EVENT:
                    handleLoginRequest(ctx, (Map<String, Object>) msg.data);
                    break;
                case Protocol.MSG_CONNECT_REQUEST:
                    handleConnectRequest(ctx, (Map<String, Object>) msg.data);
                    break;

                case Protocol.MSG_PLAYER_MOVE:
                    handlePlayerMove(ctx, (Map<String, Object>) msg.data);
                    break;

                case Protocol.MSG_CHAT_MESSAGE:
                    handleChatMessage(ctx, (Map<String, Object>) msg.data);
                    break;
                case Protocol.MSG_HEARTBEAT:
                    handleHeartbeatMessage(ctx, (Map<String, Object>) msg.data);
                    break;
                case Protocol.MSG_MAP_DATA_REQUEST://加载地图
                    handleMapDataRequest(ctx, (Map<String, Object>) msg.data);
                    break;
                case Protocol.MSG_PATH_REQUEST://寻路
                    handlePathRequest(ctx, (Map<String, Object>) msg.data, sessionId);
                    break;
                case Protocol.MSG_TALK_TO_NPC: //客户端与NCP对话请求
                    handleTalkToNPC(ctx, (Map<String, Object>) msg.data);
                    break;
                default:
                    logger.warn("未知消息类型: {}", msg.type);
            }

        } catch (Exception e) {
            logger.error("处理消息失败: {}", message, e);
        }
    }

    private void handleLoginRequest(ChannelHandlerContext ctx, Map<String, Object> data) {
        String playerName = (String) data.get("playerName");
        String password = (String) data.get("password");
        String sessionId = ctx.channel().id().asShortText();

        logger.info("登录请求: name={}, password={}, session={}", playerName, password, sessionId);

        // 1. 验证参数
        if (playerName == null || playerName.trim().isEmpty()) {
            sendLoginResponse(ctx, false, "玩家名不能为空", null);
            return;
        }

        if (password == null || password.trim().isEmpty()) {
            sendLoginResponse(ctx, false, "密码不能为空", null);
            return;
        }

        // 2. 验证密码（简单示例，实际应查数据库）
        if (!validatePassword(playerName, password)) {
            logger.warn("登录失败: 密码错误 - {}", playerName);
            sendLoginResponse(ctx, false, "密码错误", null);
            return;
        }

        // 3. 检查是否已在线
        if (isPlayerOnline(playerName)) {
            logger.warn("登录失败: 玩家已在线 - {}", playerName);
            sendLoginResponse(ctx, false, "该账号已在其他地方登录", null);
            return;
        }

        // 4. 获取或创建玩家
        Player player = world.getPlayerByName(playerName);
        if (player == null) {
            // 新玩家，创建角色
            player = world.createPlayer(playerName);
            if (player == null) {
                logger.error("登录失败: 创建玩家失败 - {}", playerName);
                sendLoginResponse(ctx, false, "创建角色失败", null);
                return;
            }
            logger.info("创建新玩家: {}", playerName);
        } else {
            logger.info("加载已有玩家: {}", playerName);
        }

        // 6. 建立会话映射
        playerSessions.put(sessionId, player.getId());

        logger.info("登录成功: {} (ID: {}), 位置: ({},{})",
                player.getName(), player.getId(), player.getX(), player.getY());

        // 8. 发送登录成功响应
        sendLoginResponse(ctx, true, "登录成功", player);

        // 9. 广播玩家上线
        broadcastPlayerJoin(player);

        // 10. 发送世界状态
        sendWorldState(ctx, player.getId());
    }

    /**
     * 处理地图数据请求
     */
    private void handleMapDataRequest(ChannelHandlerContext ctx, Map<String, Object> data) {
        try {
            // 获取请求的地图ID
            Integer mapId = (Integer) data.get("mapId");
            if (mapId == null) {
                mapId = 1; // 默认新手村地图
            }

            // 获取地图数据
            MapData mapData = world.getMapData(mapId);
            if (mapData == null) {
                logger.warn("地图不存在: mapId={}", mapId);
                sendMapDataError(ctx, "地图不存在");
                return;
            }

            // 构建地图数据响应
            Map<String, Object> responseData = new HashMap<>();
            responseData.put("success", true);
            responseData.put("mapId", mapData.getMapId());
            responseData.put("mapName", mapData.getMapName());
            responseData.put("width", mapData.getWidth());
            responseData.put("height", mapData.getHeight());

            // 发送地形数据（一维数组）
            int[] tileData = new int[mapData.getWidth() * mapData.getHeight()];
            for (int y = 0; y < mapData.getHeight(); y++) {
                for (int x = 0; x < mapData.getWidth(); x++) {
                    tileData[y * mapData.getWidth() + x] = mapData.getTile(x, y).getCode();
                }
            }
            responseData.put("tileData", tileData);

            // 发送NPC数据
            List<Map<String, Object>> npcList = new ArrayList<>();
            for (NPC npc : mapData.getNpcs()) {
                Map<String, Object> npcData = new HashMap<>();
                npcData.put("id", npc.getId());
                npcData.put("name", npc.getName());
                npcData.put("x", npc.getX());
                npcData.put("y", npc.getY());
                npcData.put("dialog", npc.getDialog());
                npcData.put("symbol", npc.getSymbol() != null ? npc.getSymbol() : "👤");
                npcList.add(npcData);
            }
            responseData.put("npcs", npcList);

            // 发送传送门数据（可选）
            List<Map<String, Object>> portalList = new ArrayList<>();
            for (Portal portal : mapData.getPortals()) {
                Map<String, Object> portalData = new HashMap<>();
                portalData.put("id", portal.getId());
                portalData.put("name", portal.getName());
                portalData.put("x", portal.getSourceX());
                portalData.put("y", portal.getSourceY());
                portalData.put("targetMapId", portal.getTargetMapId());
                portalData.put("targetX", portal.getTargetX());
                portalData.put("targetY", portal.getTargetY());
                portalList.add(portalData);
            }
            responseData.put("portals", portalList);

            // 发送响应
            Protocol.Message response = new Protocol.Message(Protocol.MSG_MAP_DATA_REQUEST, responseData);
            String json = Protocol.serialize(response);
            ctx.writeAndFlush(new TextWebSocketFrame(json));

            logger.info("地图数据已发送: mapId={}, 尺寸={}x{}, NPC数量={}",
                    mapId, mapData.getWidth(), mapData.getHeight(), npcList.size());

        } catch (Exception e) {
            logger.error("处理地图数据请求失败", e);
            sendMapDataError(ctx, "服务器内部错误");
        }
    }

    /**
     * 发送地图数据错误响应
     */
    private void sendMapDataError(ChannelHandlerContext ctx, String error) {
        try {
            Map<String, Object> errorData = new HashMap<>();
            errorData.put("success", false);
            errorData.put("error", error);
            Protocol.Message response = new Protocol.Message(Protocol.MSG_MAP_DATA_REQUEST, errorData);
            String json = Protocol.serialize(response);
            ctx.writeAndFlush(new TextWebSocketFrame(json));
            logger.warn("发送地图数据错误: {}", error);
        } catch (Exception e) {
            logger.error("发送错误响应失败", e);
        }
    }

    private void handleHeartbeatMessage(ChannelHandlerContext ctx, Map<String, Object> data) {

        Integer playerId = (Integer) data.get("playerId");
        Number timestamp = (Number) data.get("timestamp");
        logger.info("收到心跳消息: playerId={}, timestamp={}", playerId, timestamp);

        // 回复心跳确认消息
        try {
            Map<String, Object> responseData = new HashMap<>();
            responseData.put("playerId", playerId);
            responseData.put("timestamp", timestamp);
            Protocol.Message response = new Protocol.Message(Protocol.MSG_HEARTBEAT, responseData);
            ctx.writeAndFlush(new TextWebSocketFrame(Protocol.serialize(response)));
        } catch (JsonProcessingException e) {
            logger.error("心跳响应序列化失败", e);
        }
    }

    /**
     * 玩家接入
     *
     * @param ctx
     * @param data
     */
    private void handleConnectRequest(ChannelHandlerContext ctx, Map<String, Object> data) {
        String playerName = (String) data.get("playerName");
        if (playerName == null || playerName.trim().isEmpty()) {
            playerName = "玩家" + System.currentTimeMillis() % 1000;
        }

        // 创建新玩家
        Player player = world.createPlayer(playerName);
        String sessionId = ctx.channel().id().asShortText();
        playerSessions.put(sessionId, player.getId());

        logger.info("玩家连接: {} (ID: {}), 会话: {}", playerName, player.getId(), sessionId);

        // 发送连接响应
        try {
            Protocol.Message response = Protocol.createConnectResponse(
                    player.getId(), player.getX(), player.getY());
            ctx.writeAndFlush(new TextWebSocketFrame(Protocol.serialize(response)));

            // 广播玩家加入
            broadcastPlayerJoin(player);

            // 发送当前世界状态
            sendWorldState(ctx, player.getId());

        } catch (JsonProcessingException e) {
            logger.error("序列化响应失败", e);
        }
    }

    /**
     * 处理玩家移动
     */
    private void handlePlayerMove(ChannelHandlerContext ctx, Map<String, Object> data) {
        Integer playerId = (Integer) data.get("playerId");
        Number x = (Number) data.get("x");
        Number y = (Number) data.get("y");
        Number direction = (Number) data.get("direction");

        if (playerId == null || x == null || y == null) {
            return;
        }

        Player player = world.getPlayer(playerId);
        if (player != null) {
            int targetX = x.intValue();
            int targetY = y.intValue();

            // 验证移动
            if (world.validateMove(player, targetX, targetY)) {
                player.setX(targetX);
                player.setY(targetY);
                if (direction != null) {
                    player.setDirection(direction.floatValue());
                }

                // 检查传送门
                world.checkPortal(player, targetX, targetY);

                // 广播移动信息
                broadcastPlayerMove(player);
            }
        }
    }

    private void handleTalkToNPC(ChannelHandlerContext ctx, Map<String, Object> data) {
        int playerId = (int) data.get("playerId");
        int npcId = (int) data.get("npcId");

        Player player = world.getPlayer(playerId);
        if (player == null) {
            logger.warn("玩家不存在: {}", playerId);
            return;
        }

        NPC npc = world.getNPCById(player.getMapId(), npcId);
        if (npc == null) {
            logger.warn("NPC不存在: {}", npcId);
            return;
        }

        logger.info("玩家 {} 与 NPC {} 对话", player.getName(), npc.getName());

        // 1. 检查是否有可完成的任务
        boolean questCompleted = false;
        for (PlayerQuest quest : QuestManager.getInstance().getPlayerQuests(playerId).values()) {
            if (quest.getStatus() == QuestStatus.IN_PROGRESS
                    && quest.getType() == QuestType.TALK
                    && quest.getTargetNPCId() == npcId) {

                if (QuestManager.getInstance().completeQuest(playerId, quest.getId(), player)) {
                    questCompleted = true;
                    sendQuestCompleteResponse(ctx, quest);
                    logger.info("玩家 {} 完成任务: {}", player.getName(), quest.getName());
                }
            }
        }

        // 2. 检查是否有可接取的任务
        List<QuestTemplate> availableQuests = QuestManager.getInstance()
                .getAvailableQuests(playerId, npcId);

        if (!availableQuests.isEmpty()) {
            sendAvailableQuestsResponse(ctx, availableQuests);
            logger.info("玩家 {} 有 {} 个可接取任务", player.getName(), availableQuests.size());
            return;
        }

        // 3. 如果没有可完成的任务也没有可接取的任务，发送普通对话
        if (!questCompleted) {
            sendNPCDialog(ctx, npc);
        }
    }

    private NPC getNPCById(int mapId, int npcId) {
        return world.getNPCById(mapId, npcId);
    }

    private void sendQuestCompleteResponse(ChannelHandlerContext ctx, PlayerQuest quest) {
        try {
            Map<String, Object> responseData = new HashMap<>();
            responseData.put("questId", quest.getId());
            responseData.put("questName", quest.getName());

            // 发送奖励信息
            Map<String, Object> rewards = new HashMap<>();
            for (Map.Entry<String, Object> entry : quest.getRewards().entrySet()) {
                rewards.put(entry.getKey(), entry.getValue());
            }
            responseData.put("rewards", rewards);

            Protocol.Message response = new Protocol.Message(Protocol.MSG_QUEST_COMPLETE, responseData);
            String json = Protocol.serialize(response);
            ctx.writeAndFlush(new TextWebSocketFrame(json));

            logger.info("任务完成: {} (ID: {})", quest.getName(), quest.getId());

        } catch (Exception e) {
            logger.error("发送任务完成响应失败", e);
        }
    }
    
   // 接受任务请求
    private void handleAcceptQuest(ChannelHandlerContext ctx, Map<String, Object> data) {
        int playerId = (int) data.get("playerId");
        String questId = (String) data.get("questId");
        int npcId = (int) data.get("npcId");

        // ✅ 现在可以正常获取了
        QuestTemplate template = QuestManager.getInstance().getQuestTemplate(questId);

        if (template == null) {
            sendAcceptQuestResponse(ctx, false, questId, "任务不存在");
            return;
        }

        if (template.getTargetNPCId() != npcId) {
            sendAcceptQuestResponse(ctx, false, questId, "NPC不正确");
            return;
        }

        boolean success = QuestManager.getInstance().acceptQuest(playerId, questId, npcId);

        if (success) {
            sendAcceptQuestResponse(ctx, true, questId, template.getName());
            logger.info("玩家 {} 接受了任务: {}", playerId, template.getName());
        } else {
            sendAcceptQuestResponse(ctx, false, questId, "接受任务失败");
        }
    }

    private void sendAcceptQuestResponse(ChannelHandlerContext ctx, boolean success,
            String questId, String questName) {
        try {
            Map<String, Object> responseData = new HashMap<>();
            responseData.put("success", success);
            responseData.put("questId", questId);
            if (success) {
                responseData.put("questName", questName);
            } else {
                responseData.put("error", questName);
            }

            Protocol.Message response = new Protocol.Message(Protocol.MSG_ACCEPT_QUEST_RESPONSE, responseData);
            String json = Protocol.serialize(response);
            ctx.writeAndFlush(new TextWebSocketFrame(json));

        } catch (Exception e) {
            logger.error("发送接受任务响应失败", e);
        }
    }

    private void sendAvailableQuestsResponse(ChannelHandlerContext ctx, List<QuestTemplate> quests) {
        try {
            List<Map<String, Object>> questList = new ArrayList<>();
            for (QuestTemplate quest : quests) {
                Map<String, Object> q = new HashMap<>();
                q.put("questId", quest.getId());
                q.put("questName", quest.getName());
                q.put("description", quest.getDescription());
                q.put("targetNPCId", quest.getTargetNPCId());
                questList.add(q);
            }

            Map<String, Object> responseData = new HashMap<>();
            responseData.put("quests", questList);

            Protocol.Message response = new Protocol.Message(Protocol.MSG_AVAILABLE_QUESTS, responseData);
            String json = Protocol.serialize(response);
            ctx.writeAndFlush(new TextWebSocketFrame(json));

            logger.info("发送可接取任务列表，数量: {}", quests.size());

        } catch (Exception e) {
            logger.error("发送可接取任务列表失败", e);
        }
    }

    private void sendNPCDialog(ChannelHandlerContext ctx, NPC npc) {
        try {
            Map<String, Object> responseData = new HashMap<>();
            responseData.put("npcId", npc.getId());
            responseData.put("npcName", npc.getName());
            responseData.put("dialog", npc.getDialog());
            responseData.put("symbol", npc.getSymbol());

            Protocol.Message response = new Protocol.Message(Protocol.MSG_NPC_DIALOG, responseData);
            String json = Protocol.serialize(response);
            ctx.writeAndFlush(new TextWebSocketFrame(json));

            logger.info("发送NPC对话: {}", npc.getName());

        } catch (Exception e) {
            logger.error("发送NPC对话失败", e);
        }
    }

    private void handleChatMessage(ChannelHandlerContext ctx, Map<String, Object> data) {
        Integer playerId = (Integer) data.get("playerId");
        String message = (String) data.get("message");

        if (playerId != null && message != null) {
            Player player = world.getPlayer(playerId);
            if (player != null) {
                logger.info("聊天: {}: {}", player.getName(), message);
                // 这里可以广播聊天消息
            }
        }
    }

    private void broadcastPlayerJoin(Player player) {
        try {
            Map<String, Object> joinData = new HashMap<>();
            joinData.put("playerId", player.getId());
            joinData.put("playerName", player.getName());
            joinData.put("x", player.getX());
            joinData.put("y", player.getY());
            joinData.put("direction", player.getDirection());
            joinData.put("level", player.getLevel());

            Protocol.Message joinMsg = new Protocol.Message(Protocol.MSG_PLAYER_JOIN, joinData);
            String json = Protocol.serialize(joinMsg);

            // 广播给同一地图的所有玩家
            for (Map.Entry<String, Integer> entry : playerSessions.entrySet()) {
                String sessionId = entry.getKey();
                Integer otherPlayerId = entry.getValue();

                if (otherPlayerId.equals(player.getId())) {
                    continue;
                }

                Player otherPlayer = world.getPlayer(otherPlayerId);
                if (otherPlayer != null && otherPlayer.getMapId() == player.getMapId()) {
                    ChannelHandlerContext otherCtx = sessionContexts.get(sessionId);
                    if (otherCtx != null) {
                        otherCtx.writeAndFlush(new TextWebSocketFrame(json));
                    }
                }
            }

            logger.info("广播玩家加入: {} (ID: {})", player.getName(), player.getId());

        } catch (Exception e) {
            logger.error("广播玩家加入失败", e);
        }
    }

    private void broadcastPlayerMove(Player player) {
        // 在实际项目中，这里会广播给附近玩家
        logger.debug("广播玩家移动: {} ({}, {})",
                player.getName(), player.getX(), player.getY());
    }

    /**
     * 发送世界状态给客户端
     */
    private void sendWorldState(ChannelHandlerContext ctx, int excludePlayerId) {
        try {
            // 获取发送消息的玩家
            String sessionId = ctx.channel().id().asShortText();
            Integer playerId = playerSessions.get(sessionId);

            if (playerId == null) {
                return;
            }

            Player player = world.getPlayer(playerId);
            if (player == null) {
                return;
            }
            List<Player> playersInMap = world.getPlayersInMap(player.getMapId());
            List<Map<String, Object>> playerList = new ArrayList<>();

            for (Player p : playersInMap) {
                if (p.getId() == excludePlayerId) {
                    continue;
                }

                Map<String, Object> playerData = new HashMap<>();
                playerData.put("id", p.getId());
                playerData.put("name", p.getName());
                playerData.put("x", p.getX());
                playerData.put("y", p.getY());
                playerData.put("direction", p.getDirection());
                playerData.put("level", p.getLevel());
                playerList.add(playerData);
            }

            Map<String, Object> data = new HashMap<>();
            data.put("players", playerList);
            data.put("mapId", player.getMapId());

            Protocol.Message msg = new Protocol.Message(Protocol.MSG_WORLD_STATE, data);
            String json = Protocol.serialize(msg);
            ctx.writeAndFlush(new TextWebSocketFrame(json));

        } catch (Exception e) {
            logger.error("发送世界状态失败", e);
        }
    }

    private void sendError(ChannelHandlerContext ctx, String error, int ptlMsg) {
        try {
            Map<String, Object> errorData = new HashMap<>();
            errorData.put("success", false);
            errorData.put("error", error);
            Protocol.Message msg = new Protocol.Message(ptlMsg, errorData);
            ctx.writeAndFlush(new TextWebSocketFrame(Protocol.serialize(msg)));
        } catch (JsonProcessingException e) {
            logger.error("发送错误消息失败", e);
        }
    }

    @Override
    public void channelInactive(ChannelHandlerContext ctx) throws Exception {
        String sessionId = ctx.channel().id().asShortText();
        Integer playerId = playerSessions.remove(sessionId);

        if (playerId != null) {
            sessionByPlayer.remove(playerId);
            sessionContexts.remove(sessionId);
            Player player = world.removePlayer(playerId);
            if (player != null) {
                logger.info("玩家断开连接: {} (ID: {})", player.getName(), playerId);
                broadcastPlayerLeave(playerId);
            }
        }

        super.channelInactive(ctx);
    }

    private void broadcastPlayerLeave(int playerId) {
        try {
            Map<String, Object> leaveData = new HashMap<>();
            leaveData.put("playerId", playerId);

            Protocol.Message leaveMsg = new Protocol.Message(Protocol.MSG_PLAYER_LEAVE, leaveData);
            String json = Protocol.serialize(leaveMsg);

            // 广播给所有玩家
            for (ChannelHandlerContext ctx : sessionContexts.values()) {
                ctx.writeAndFlush(new TextWebSocketFrame(json));
            }

        } catch (Exception e) {
            logger.error("广播玩家离开失败", e);
        }
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        logger.error("连接异常", cause);
        ctx.close();
    }

    /**
     * 处理路径请求
     */
    private void handlePathRequest(ChannelHandlerContext ctx, Map<String, Object> data, String sessionId) {
        try {
            int startX = (int) data.get("startX");
            int startY = (int) data.get("startY");
            int targetX = (int) data.get("targetX");
            int targetY = (int) data.get("targetY");

            // 获取当前玩家的地图
            Integer playerId = playerSessions.get(sessionId);
            logger.info("收到{}的路径请求: ({},{}) -> ({},{})", sessionId, startX, startY, targetX, targetY);
            if (playerId == null) {
                sendPathError(ctx, "玩家未找到");
                return;
            }

            Player player = world.getPlayer(playerId);
            if (player == null) {
                sendPathError(ctx, "玩家不存在");
                return;
            }

            // 获取当前地图
            MapData mapData = world.getMapData(player.getMapId());
            if (mapData == null) {
                sendPathError(ctx, "地图不存在");
                return;
            }

            // 检查目标是否可行走
            if (!mapData.isWalkable(targetX, targetY)) {
                sendPathError(ctx, "目标位置不可行走");
                return;
            }

            // 执行寻路
            List<int[]> path = PathFinder.findPath(mapData, startX, startY, targetX, targetY);

            if (path == null || path.isEmpty()) {
                sendPathError(ctx, "无法找到路径");
                return;
            }

            // 构建路径响应
            List<Map<String, Integer>> pathPoints = new ArrayList<>();
            for (int[] point : path) {
                Map<String, Integer> pointMap = new HashMap<>();
                pointMap.put("x", point[0]);
                pointMap.put("y", point[1]);
                pathPoints.add(pointMap);
            }

            Map<String, Object> responseData = new HashMap<>();
            responseData.put("success", true);
            responseData.put("path", pathPoints);

            Protocol.Message response = new Protocol.Message(Protocol.MSG_PATH_RESPONSE, responseData);
            String json = Protocol.serialize(response);
            ctx.writeAndFlush(new TextWebSocketFrame(json));

            logger.info("路径计算完成，共 {} 步", path.size());

        } catch (Exception e) {
            logger.error("处理路径请求失败", e);
            sendPathError(ctx, "服务器内部错误");
        }
    }

    /**
     * 发送路径错误响应
     */
    private void sendPathError(ChannelHandlerContext ctx, String error) {
        try {
            Map<String, Object> errorData = new HashMap<>();
            errorData.put("success", false);
            errorData.put("error", error);
            Protocol.Message response = new Protocol.Message(Protocol.MSG_PATH_RESPONSE, errorData);
            String json = Protocol.serialize(response);
            ctx.writeAndFlush(new TextWebSocketFrame(json));
            logger.warn("路径请求失败: {}", error);
        } catch (Exception e) {
            logger.error("发送错误响应失败", e);
        }
    }

    private void sendLoginResponse(ChannelHandlerContext ctx, boolean b, String note, Player player) {
        try {
            Map<String, Object> responseData = new HashMap<>();
            responseData.put("success", true);
            responseData.put("playerId", player.getId());
            responseData.put("playerName", player.getName());
            responseData.put("level", player.getLevel());
            responseData.put("exp", player.getExperience());
            responseData.put("mapId", player.getMapId());
            responseData.put("x", player.getX());
            responseData.put("y", player.getY());
            responseData.put("direction", player.getDirection());
            Protocol.Message response = new Protocol.Message(Protocol.MSG_LOGIN_EVENT, responseData);
            String json = Protocol.serialize(response);
            ctx.writeAndFlush(new TextWebSocketFrame(json));
        } catch (Exception e) {
            logger.error("处理返回玩家登录请求失败", e);
        }
    }
//数据库验证

    private boolean validatePassword(String playerName, String password) {
        return true;
    }

    private boolean isPlayerOnline(String playerName) {
        for (Map.Entry<String, Integer> entry : playerSessions.entrySet()) {
            Player player = world.getPlayer(entry.getValue());
            if (player != null && player.getName().equals(playerName)) {
                return true;
            }
        }
        return false;
    }

}
