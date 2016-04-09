


--脚本加载标志
已经加载战斗脚本 = false
脚本_战斗结束 = false

--开始
function 脚本入口()

	if 已经加载战斗脚本 == false then
		调试信息("加载战斗脚本")
		dofile("任务-战斗.lua")
		已经加载战斗脚本 = true
	end

	检测离线()
	关闭无关窗口()
	等待标识消失()
	处理死亡()

	状态 = 获取玩家状态()
	if 状态 == 战斗状态 then

		脚本_战斗结束 = true
		if 存在图片("战斗-菜单2", 0.8) then
			战斗回调()
		elseif 存在图片("宠物战斗菜单", 0.8) then
			宠物战斗回调()
		end

	elseif 状态 == 正常状态 then

		if 脚本_战斗结束 == true then
			加血()
			脚本_战斗结束 = false
		end
		
		if 当前城市("长安城") then
			找驿站老板()
		elseif 当前城市("大唐国境") then
			前往("小地图-大唐境外")
		elseif 当前城市("大唐境外") then
			点击小地图(100, 30)
			点击小地图(30, 30)		--来回走动
		end

	elseif 状态 == 动画状态 then
		点击图片("跳过动画")
	else
		调试信息("未知状态..")
	end

	关闭无关窗口()

end