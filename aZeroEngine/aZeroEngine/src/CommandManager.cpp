#include "CommandManager.h"

void CommandManager::returnGraphicsHandle(std::shared_ptr<CommandContext> context)
{
	context->stopRecording();

	std::lock_guard<std::mutex> lock(m_graphicsMutex);
	m_freeGraphicsContexts.push_back(context);
}

void CommandManager::returnCopyHandle(std::shared_ptr<CommandContext> context)
{
	context->stopRecording();

	std::lock_guard<std::mutex> lock(m_copyMutex);
	m_freeCopyContexts.push_back(context);
}

void CommandManager::returnComputeHandle(std::shared_ptr<CommandContext> context)
{
	context->stopRecording();

	std::lock_guard<std::mutex> lock(m_computeMutex);
	m_freeComputeContexts.push_back(context);
}

CommandManager::CommandManager(ID3D12Device* device, UINT numGraphicsContexts, UINT numCopyContexts, UINT numComputeContexts)
{
	m_directQueue.init(device, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAG_NONE);

#ifdef _DEBUG
	std::wstring name = L"";
	name = L"Direct Queue";
	m_directQueue.getQueue()->SetName(name.c_str());
#endif // _DEBUG

	for (int i = 0; i < numGraphicsContexts; i++)
	{
		m_freeGraphicsContexts.push_back(std::make_shared<CommandContext>(device, D3D12_COMMAND_LIST_TYPE_DIRECT));

#ifdef _DEBUG
		name = L"Graphics ";
		name += L"Allocator " + std::to_wstring(i);
		m_freeGraphicsContexts.back()->m_allocator->SetName(name.c_str());

		name = L"Graphics ";
		name += L"List " + std::to_wstring(i);
		m_freeGraphicsContexts.back()->m_commandList->SetName(name.c_str());
#endif // _DEBUG
	}

	m_copyQueue.init(device, D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAG_NONE);

#ifdef _DEBUG
	name = L"Copy Queue";
	m_copyQueue.getQueue()->SetName(name.c_str());
#endif // _DEBUG

	for (int i = 0; i < numCopyContexts; i++)
	{
		m_freeCopyContexts.push_back(std::make_shared<CommandContext>(device, D3D12_COMMAND_LIST_TYPE_COPY));

#ifdef _DEBUG
		name = L"Copy ";
		name += L"Allocator " + std::to_wstring(i);
		m_freeCopyContexts.back()->m_allocator->SetName(name.c_str());

		name = L"Copy ";
		name += L"List " + std::to_wstring(i);
		m_freeCopyContexts.back()->m_commandList->SetName(name.c_str());
#endif // _DEBUG
	}

	m_computeQueue.init(device, D3D12_COMMAND_LIST_TYPE_COMPUTE, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAG_NONE);

#ifdef _DEBUG
	name = L"Compute Queue";
	m_computeQueue.getQueue()->SetName(name.c_str());
#endif // _DEBUG

	for (int i = 0; i < numComputeContexts; i++)
	{
		m_freeComputeContexts.push_back(std::make_shared<CommandContext>(device, D3D12_COMMAND_LIST_TYPE_COMPUTE));

#ifdef _DEBUG
		name = L"Compute ";
		name += L"Allocator " + std::to_wstring(i);
		m_freeComputeContexts.back()->m_allocator->SetName(name.c_str());

		name = L"Compute ";
		name += L"List " + std::to_wstring(i);
		m_freeComputeContexts.back()->m_commandList->SetName(name.c_str());
#endif // _DEBUG
	}
}

void CommandManager::flushCPU(bool resetContexts)
{
	m_computeQueue.flushCPU(m_computeQueue.getLastSignalValue());
	m_copyQueue.flushCPU(m_copyQueue.getLastSignalValue());
	m_directQueue.flushCPU(m_directQueue.getLastSignalValue());

	if (resetContexts)
	{
		{
			std::lock_guard<std::mutex> lock(m_graphicsMutex);
			for (std::shared_ptr<CommandContext> context : m_freeGraphicsContexts)
			{
				if (context->hasRecorded())
				{
					context->reset();
				}
			}
		}

		{
			std::lock_guard<std::mutex> lock(m_copyMutex);
			for (std::shared_ptr<CommandContext> context : m_freeCopyContexts)
			{
				if (context->hasRecorded())
				{
					context->reset();
				}
			}
		}

		{
			std::lock_guard<std::mutex> lock(m_computeMutex);
			for (std::shared_ptr<CommandContext> context : m_freeComputeContexts)
			{
				if (context->hasRecorded())
				{
					context->reset();
				}
			}
		}
	}
}

GraphicsContextHandle CommandManager::getGraphicsContext()
{
	std::shared_ptr<CommandContext> tempContext = nullptr;

	{
		std::lock_guard<std::mutex> lock(m_graphicsMutex);
		tempContext = m_freeGraphicsContexts.front();
		m_freeGraphicsContexts.pop_front();
	}

	tempContext->startRecording();

	return GraphicsContextHandle(this, tempContext);
}

CopyContextHandle CommandManager::getCopyContext()
{
	std::shared_ptr<CommandContext> tempContext = nullptr;

	{
		std::lock_guard<std::mutex> lock(m_copyMutex);
		tempContext = m_freeCopyContexts.front();
		m_freeCopyContexts.pop_front();
	}

	tempContext->startRecording();

	return CopyContextHandle(this, tempContext);
}

ComputeContextHandle CommandManager::getComputeContext()
{
	std::shared_ptr<CommandContext> tempContext = nullptr;

	{
		std::lock_guard<std::mutex> lock(m_copyMutex);
		tempContext = m_freeComputeContexts.front();
		m_freeComputeContexts.pop_front();
	}

	tempContext->startRecording();

	return ComputeContextHandle(this, tempContext);
}

UINT64 CommandManager::executeContext(GraphicsContextHandle& contextHandle)
{
	contextHandle.m_context->stopRecording();

	UINT64 fenceValue = m_directQueue.execute(contextHandle.getList());

	contextHandle.setLastFence(fenceValue);
	contextHandle.m_context->startRecording();

	return fenceValue;
}

UINT64 CommandManager::executeContexts(GraphicsContextHandle* contextHandles, UINT numHandles)
{
	std::vector<ID3D12CommandList*> commandLists;
	commandLists.reserve(numHandles);
	for (int i = 0; i < numHandles; i++)
	{
		contextHandles[i].m_context->stopRecording();
		commandLists.emplace_back(contextHandles[i].m_context->m_commandList.Get());
	}

	UINT64 fenceValue = m_directQueue.execute(commandLists[0], numHandles);

	for (int i = 0; i < numHandles; i++)
	{
		contextHandles[i].setLastFence(fenceValue);
		contextHandles[i].m_context->startRecording();
	}

	return fenceValue;
}

UINT64 CommandManager::executeContext(CopyContextHandle& contextHandle)
{
	contextHandle.m_context->stopRecording();

	UINT64 fenceValue = m_copyQueue.execute(contextHandle.getList());

	contextHandle.setLastFence(fenceValue);
	contextHandle.m_context->startRecording();

	return fenceValue;
}

UINT64 CommandManager::executeContexts(CopyContextHandle* contextHandles, UINT numHandles)
{
	std::vector<ID3D12CommandList*> commandLists;
	commandLists.reserve(numHandles);
	for (int i = 0; i < numHandles; i++)
	{
		contextHandles[i].m_context->stopRecording();
		commandLists.emplace_back(contextHandles[i].m_context->m_commandList.Get());
	}

	UINT64 fenceValue = m_copyQueue.execute(commandLists[0], numHandles);

	for (int i = 0; i < numHandles; i++)
	{
		contextHandles[i].setLastFence(fenceValue);
		contextHandles[i].m_context->startRecording();
	}

	return fenceValue;
}

UINT64 CommandManager::executeContext(ComputeContextHandle& contextHandle)
{
	contextHandle.m_context->stopRecording();

	UINT64 fenceValue = m_computeQueue.execute(contextHandle.getList());

	contextHandle.setLastFence(fenceValue);
	contextHandle.m_context->startRecording();

	return fenceValue;
}

UINT64 CommandManager::executeContexts(ComputeContextHandle* contextHandles, UINT numHandles)
{
	std::vector<ID3D12CommandList*> commandLists;
	commandLists.reserve(numHandles);
	for (int i = 0; i < numHandles; i++)
	{
		contextHandles[i].m_context->stopRecording();
		commandLists.emplace_back(contextHandles[i].m_context->m_commandList.Get());
	}

	UINT64 fenceValue = m_computeQueue.execute(commandLists[0], numHandles);

	for (int i = 0; i < numHandles; i++)
	{
		contextHandles[i].setLastFence(fenceValue);
		contextHandles[i].m_context->startRecording();
	}

	return fenceValue;
}
