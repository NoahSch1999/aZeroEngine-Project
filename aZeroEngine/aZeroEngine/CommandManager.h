#include <mutex>
#include <list>

#include "CommandQueue.h"
#include "GraphicsContextHandle.h"
#include "CopyContextHandle.h"
#include "ComputeContextHandle.h"

/** @brief Generates, contains, and distribute GraphicsContextHandle, CopyContextHandle, and ComputeContextHandle instances.
Contexts may be requested using CommandManager::GetXContext() in a thread safe manner.
Contexts may only be used on the thread they are requested on.
Contexts are executed using CommandManager::ExecuteContext() in a thread safe manner.
Each internal lists of available contexts inside the class has its own mutex which means that threads can request context instances simulataneously as long as they are of a different context type.
*/
class CommandManager
{
	friend class CommandContext;
	friend class GraphicsContextHandle;
	friend class CopyContextHandle;
	friend class ComputeContextHandle;

private:
	CommandQueue m_directQueue;
	std::mutex m_graphicsMutex;
	std::list<std::shared_ptr<CommandContext>> m_freeGraphicsContexts;
	void ReturnGraphicsHandle(std::shared_ptr<CommandContext> context);

	CommandQueue m_copyQueue;
	std::mutex m_copyMutex;
	std::list<std::shared_ptr<CommandContext>> m_freeCopyContexts;
	void ReturnCopyHandle(std::shared_ptr<CommandContext> context);

	CommandQueue m_computeQueue;
	std::mutex m_computeMutex;
	std::list<std::shared_ptr<CommandContext>> m_freeComputeContexts;
	void ReturnComputeHandle(std::shared_ptr<CommandContext> context);

public:
	CommandManager() = default;
	~CommandManager() = default;

	/**Generates the specified number of CommandContext handles.
	@param device The ID3D12Device for DirectX12 resource creation
	@param numGraphicsContexts Number of GraphicsContextHandle instances for direct CommandQueue work
	@param numCopyContexts Number of CopyContextHandle instances for copy CommandQueue work
	@param numComputeContexts Number of ComputeContextHandle instances for compute CommandQueue work
	*/
	CommandManager(ID3D12Device* device, UINT numGraphicsContexts, UINT numCopyContexts, UINT numComputeContexts);

	/**Returns a reference of the CommandQueue instance for direct commands.
	@return CommandQueue&
	*/
	CommandQueue& GetGraphicsQueue() { return m_directQueue; }

	/**Returns a reference of the CommandQueue instance for copy commands.
	@return CommandQueue&
	*/
	CommandQueue& GetCopyQueue() { return m_copyQueue; }

	/**Returns a reference of the CommandQueue instance for compute commands.
	@return CommandQueue&
	*/
	CommandQueue& GetComputeQueue() { return m_computeQueue; }


	// ----
	// TO DO!
	// REMOVE and replace with Context to Context wait
	// 
	// Adds GPU-side wait for the graphics queue
	void GPUFlushGraphics() { m_directQueue.WaitForFence(m_directQueue.GetLastSignalValue()); }

	// Adds GPU-side wait for the copy queue
	void GPUFlushCopy() { m_copyQueue.WaitForFence(m_copyQueue.GetLastSignalValue()); }

	// Adds GPU-side wait for the compute queue
	void GPUFlushCompute() { m_computeQueue.WaitForFence(m_computeQueue.GetLastSignalValue()); }

	void WaitForCopy() { m_directQueue.WaitForOther(m_copyQueue, m_copyQueue.GetLastSignalValue()); }
	// ----

	/**Flushes the CPU and forces it to wait for all CommandQueue work.
	@return void
	*/
	void CPUFlush();

	/**Returns a handle to a GraphicsContextHandle instance which can be used to record direct queue commands with.
	* This method is thread safe.
	@return GraphicsContextHandle
	*/
	GraphicsContextHandle GetGraphicsContext();

	/**Executes the GraphicsContextHandle in the argument list and returns the fence value for the execution.
	* It also enables the input GraphicsContextHandle to be reused and record more commands with.
	* This method is thread safe as long as the input GraphicsContextHandle is thread local.
	@param contextHandle The GraphicsContextHandle which should have it's recorded commands executed
	@return UINT64
	*/
	UINT64 ExecuteContext(GraphicsContextHandle& contextHandle);

	/**Executes the array of GraphicsContextHandle instances in the argument list and returns the fence value for the execution.
	* It also enables the input GraphicsContextHandle instances to be reused and record more commands with.
	* This method is thread safe as long as the input GraphicsContextHandle is thread local.
	@param contextHandles A pointer to the start of an array of GraphicsContextHandle instances which should have it's recorded commands executed
	@param numHandles Number of handles in the array that the pointer references.
	@return UINT64
	*/
	UINT64 ExecuteContext(GraphicsContextHandle* contextHandles, UINT numHandles);

	/**Returns a handle to a CopyContextHandle instance which can be used to record copy queue commands with.
	* This method is thread safe.
	@return CopyContextHandle
	*/
	CopyContextHandle GetCopyContext();

	/**Executes the CopyContextHandle in the argument list and returns the fence value for the execution.
	* It also enables the input CopyContextHandle to be reused and record more commands with.
	* This method is thread safe as long as the input CopyContextHandle is thread local.
	@param contextHandle The CopyContextHandle which should have it's recorded commands executed
	@return UINT64
	*/
	UINT64 ExecuteContext(CopyContextHandle& contextHandle);

	/**Executes the array of CopyContextHandle instances in the argument list and returns the fence value for the execution.
	* It also enables the input CopyContextHandle instances to be reused and record more commands with.
	* This method is thread safe as long as the input CopyContextHandle is thread local.
	@param contextHandles A pointer to the start of an array of CopyContextHandle instances which should have it's recorded commands executed
	@param numHandles Number of handles in the array that the pointer references.
	@return UINT64
	*/
	UINT64 ExecuteContext(CopyContextHandle* contextHandles, UINT numHandles);

	/**Returns a handle to a ComputeContextHandle instance which can be used to record compute queue commands with.
	* This method is thread safe.
	@return ComputeContextHandle
	*/
	ComputeContextHandle GetComputeContext();

	/**Executes the ComputeContextHandle in the argument list and returns the fence value for the execution.
	* It also enables the input ComputeContextHandle to be reused and record more commands with.
	* This method is thread safe as long as the input ComputeContextHandle is thread local.
	@param contextHandle The ComputeContextHandle which should have it's recorded commands executed
	@return UINT64
	*/
	UINT64 ExecuteContext(ComputeContextHandle& contextHandle);

	/**Executes the array of ComputeContextHandle instances in the argument list and returns the fence value for the execution.
	* It also enables the input ComputeContextHandle instances to be reused and record more commands with.
	* This method is thread safe as long as the input ComputeContextHandle is thread local.
	@param contextHandles A pointer to the start of an array of ComputeContextHandle instances which should have it's recorded commands executed
	@param numHandles Number of handles in the array that the pointer references.
	@return UINT64
	*/
	UINT64 ExecuteContext(ComputeContextHandle* contextHandles, UINT numHandles);

};

inline GraphicsContextHandle::~GraphicsContextHandle() noexcept
{
	if (m_context)
		m_manager->ReturnGraphicsHandle(m_context);
}

inline CopyContextHandle::~CopyContextHandle() noexcept
{
	if (m_context)
		m_manager->ReturnCopyHandle(m_context);
}

inline ComputeContextHandle::~ComputeContextHandle() noexcept
{
	if (m_context)
		m_manager->ReturnComputeHandle(m_context);
}

inline GraphicsContextHandle::GraphicsContextHandle(GraphicsContextHandle&& other) noexcept
{
	m_context = other.m_context;
	m_manager = other.m_manager;
	other.m_context = nullptr;
}

inline CopyContextHandle::CopyContextHandle(CopyContextHandle&& other) noexcept
{
	m_context = other.m_context;
	m_manager = other.m_manager;
	other.m_context = nullptr;
}

inline ComputeContextHandle::ComputeContextHandle(ComputeContextHandle&& other) noexcept
{
	m_context = other.m_context;
	m_manager = other.m_manager;
	other.m_context = nullptr;
}