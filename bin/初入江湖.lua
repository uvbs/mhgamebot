

--没有匹配到任务的时候, 但是有可点击的下划线时调用这个函数
function 通用_江湖()
	调试信息("默认的任务处理")

	延迟(1)
	等待标识消失()
	关闭无关窗口()
    点击下划线()

	隐藏玩家()
	

	if 存在图片("任务NPC标识", 0.8) then
		调试信息("通过任务处理中检测到任务NPC标识")
		x, y = 获得图片位置("任务NPC标识", 0.5)
		if x < 200 then
			移动鼠标到(x, y + 70)
			确保接触到NPC()
			按键("ALT+A")
			点击()
		else	
			x, y = 获得图片位置("任务NPC标识", 0.5)	
			移动鼠标到(x, y + 70)
			确保接触到NPC()
			点击()  --对话
			点击对话框("让我再想一想", 0, -20)
		end

		return
	elseif 存在图片("战斗", 0.5) then
		x, y = 获得图片位置("战斗", 0.5)
		if x < 200 then
			移动鼠标到(x, y + 70)
			确保接触到NPC()
			按键("ALT+A")
			点击()
		else	
			x, y = 获得图片位置("战斗", 0.5)	
			移动鼠标到(x, y + 70)
			确保接触到NPC()
			点击()  --对话
			
		end
	
	end

end


已经加载战斗脚本 = false
脚本战斗结束 = false

function 脚本入口()

	关闭无关窗口()
	--加血()

	if 已经加载战斗脚本 == false then
		调试信息("加载战斗脚本")
		dofile("除暴-战斗.lua")
		已经加载战斗脚本 = true
	end


	状态 = 获取玩家状态()
	if 状态 == 战斗状态 then
		脚本战斗结束 = true
		if 存在图片("战斗-菜单2", 0.8) then
			战斗回调()
		elseif 存在图片("宠物战斗菜单", 0.8) then
			宠物战斗回调()
		else
			
		end

	elseif 状态 == 正常状态 then

		if 脚本战斗结束 then
			加血()
			脚本战斗结束 = false
		end

		if 当前队长() then
		
			if 存在任务("长寿村") then
				if 当前城市("长寿村") then
					通用_江湖()
				else
					前往("长寿村")
				end
			elseif 存在任务("傲来国") then
				if 当前城市("长安城") then
					前往("傲来国")
				elseif 当前城市("傲来国") then
					通用_江湖()
				end
			elseif 存在任务("长寿郊外") then
				if 当前城市("长寿郊外") then
					通用_江湖()
				else
					前往("小地图-长寿郊外")
				end
			elseif 存在任务("朱紫国") then
				if 当前城市("朱紫国") then
					通用_江湖()
				else
					前往("朱紫国")
				end
	
			elseif 存在任务("西梁女国") then
				if 当前城市("西梁女国") then
					通用_江湖()
				else
					前往("西梁女国")
				end

			elseif 存在任务("江南野外") then
				if 当前城市("江南野外") then
					通用_江湖()
				else
					前往("小地图-江南野外")
				end
			elseif 存在任务("宝象国") then
				if 当前城市("宝象国") then
					通用_江湖()
				else
					前往("宝象国")
				end
			elseif 存在任务("花果山") then

				if 当前城市("花果山") then
					通用_江湖()
				else
					前往("花果山")
				end

			elseif 存在任务("建邺城") then
				if 当前城市("建邺城") then
					通用_江湖()
				else
					前往("建邺城")
				end
			elseif 存在任务("东海湾") then
				if 当前城市("东海湾") then
					通用_江湖()
				else
					前往("东海湾")
				end
			else
				调试信息("没有找到任务, 开始接任务")
				
				if 当前城市("长安城") then
					
					点击小地图(414, 70)
					--对话(30, 0)
					隐藏玩家()
					x, y = 获得图片位置("杜少海标识")
					移动鼠标到(x-157, y + 155) 
					确保接触到NPC()
					点击(0, 0)
					点击对话框("告诉我怎么做吧")
				else
					装备物品("红色合成旗")
					点击图片("小地图/长安城-布")
				end

			end
		
		end

	end






end


