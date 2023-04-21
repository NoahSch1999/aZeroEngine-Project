#pragma once
#include "ECSBase.h"
#include "TreeHierarchy.h"

// NOTE! Add so lights are local to the same entities transform as well...
/** @brief The system used for parenting and managing the parenting calculations of Entity objects and their components.
* All Entity objects with a registered Transform component are bound to this system.
*/
class ParentSystem : public ECSystem
{
private:
	TreeHierearchy hierarchy;
	std::set<int> updateNodeIDs;

	// NOTE! Make this non-recursive to be more efficient...
	void CalcBasisParents(TreeHierearchy::Node* _parent, Matrix& _matrix)
	{
		if (!_parent->parent)
			return;

		Entity tempEnt;
		tempEnt.id = _parent->parent->id;
		Transform* parentTf = componentManager.GetComponentFast<Transform>(tempEnt);
		_matrix *= parentTf->GetLocalMatrix();

		CalcBasisParents(_parent->parent, _matrix);
	}

	// NOTE! Make this non-recursive to be more efficient...
	//void UpdateWorldChildren(TreeHierearchy::Node* _current)
	//{
	//	// Get parent transform
	//	Entity tempEnt;
	//	tempEnt.id = _current->parent->id;
	//	Transform* parentTf = componentManager.GetComponentFast<Transform>(tempEnt);

	//	// Get current transform
	//	tempEnt.id = _current->id;
	//	Transform* currentTf = componentManager.GetComponentFast<Transform>(tempEnt);

	//	// Update current transform using parents world matrix
	//	currentTf->SetWorldMatrix(currentTf->GetLocalMatrix() * parentTf->GetWorldMatrix());

	//	// End recursion for this branch if the node doesn't have a child
	//	// Otherwise recursively update each child in the branch
	//	for (auto child : _current->children)
	//		UpdateWorldChildren(child);
	//}

public:

	/**Dependency injects the ComponentManager object. This constructor should only be called indirectly by SystemManager::RegisterSystem().
	@param _componentManager The ComponentManager to dependency inject and use when retrieving components for the object's method's
	*/
	ParentSystem(ComponentManager& _componentManager)
		:ECSystem(_componentManager)
	{
		componentMask.set(false);
		componentMask.set(COMPONENTENUM::TRANSFORM, true);
	}

	/**Overridden ECSystem::Bind() which is called whenever a Transform component is registered to an Entity.
	* It adds the input Entity to the internal TreeHierearchy object managed by the ParentSystem.
	@param _entity The Entity to bind.
	@return bool TRUE: Entity was bound, FALSE: Entity was not bound
	*/
	virtual bool Bind(const Entity& _entity) override
	{
		if (ECSystem::Bind(_entity))
		{
			hierarchy.Add(_entity.id);
			return true;
		}

		return false;
	}

	/**Overridden ECSystem::UnBind() which is called whenever a Transform component is unregistered from an Entity.
	* It adds the removes Entity to the internal TreeHierearchy object managed by the ParentSystem.
	@param _entity The Entity to unbind.
	@return bool TRUE: Entity was unbound, FALSE: Entity was not unbound
	*/
	virtual bool UnBind(const Entity& _entity) override
	{
		if (updateNodeIDs.contains(_entity.id))
		{
			updateNodeIDs.erase(_entity.id);
		}

		hierarchy.Remove(_entity.id);
		return ECSystem::UnBind(_entity);
	}

	/**CURRENTLY UNNUSED!
	@param N/A
	@return N/A
	*/
	virtual void Update() override
	{

	}

	/**Calculates the world matrix for the input Transform component which matches with the input Entity.
	@param _entity The Entity which is registered with the argument _transform.
	@param _transform The Transform which is registered for the argument _entity.
	@return void
	*/
	void CalculateWorld(const Entity& _entity, Transform* const _transform)
	{
		TreeHierearchy::Node* targetNode = hierarchy.GetNode(_entity.id);

		if (targetNode->parent)
		{
			Matrix parentMatrix = Matrix::Identity;
			CalcBasisParents(targetNode, parentMatrix);
			_transform->SetWorldMatrix(_transform->GetLocalMatrix() * parentMatrix);
		}
		else
		{
			_transform->SetWorldMatrix(_transform->GetLocalMatrix());
		}
	}

	/**Adds _childEntity as a child for _parentEntity.
	* Doesn't modify anything if _childEntity is a parent of _parentEntity (avoid circular parenting).
	* Calls ParentSystem::UnParent() if the _childEntity already has a parent.
	@param _parentEntity The Entity that should become the parent
	@param _childEntity The Entity that should become the child
	@return void
	*/
	void Parent(const Entity& _parentEntity, const Entity& _childEntity)
	{
		// Add so its world position becomes it's current local position...

		hierarchy.Parent(_parentEntity.id, _childEntity.id);
	}

	/**Removes _childEntity as a child for _parentEntity.
	* Doesn't modify anything if _childEntity isn't a child of _parentEntity.
	@param _parentEntity The Entity that is the parent
	@param _childEntity The Entity that is the child
	@return void
	*/
	void UnParent(const Entity& _parentEntity, const Entity& _childEntity)
	{
		// Add so its local position becomes it's current world position...

		hierarchy.UnParent(_parentEntity.id, _childEntity.id);
	}
	
	/**Returns the TreeHierarchy::Node id of the parent node for the input Entity.
	* If the Entity doesn't have a parent the method returns -1.
	@param _targetEntity The Entity to retrieve a parent for
	@return int
	*/
	int GetParentEntityID(const Entity& _targetEntity)
	{
		TreeHierearchy::Node* targetNode = hierarchy.GetNode(_targetEntity.id);

		if (targetNode->parent)
			return targetNode->parent->id;

		return -1;
	}

	/**Returns a vector to the children TreeHierarchy::Node ids directly below the input parent Entity.
	@param _targetEntity The parent Entity to retrieve children ids for
	@return std::vector<int>
	*/
	std::vector<int> GetChildrenEntityID(const Entity& _targetEntity)
	{
		TreeHierearchy::Node* targetNode = hierarchy.GetNode(_targetEntity.id);

		std::vector<int> entityIDs;
		entityIDs.reserve(targetNode->children.size());
		for (auto child : targetNode->children)
			entityIDs.emplace_back(child->id);

		return entityIDs;
	}

	/**Returns whether or not the input Entity is above a node with the input _childIDToLookFor TreeHierarchy::Node id.
	@param _branchTopEntity The Entity to check below
	@param _childIDToLookFor The TreeHierarchy::Node id to check for
	@return bool TRUE: _childIDToLookFor matches a child in the branch of _branchTopEntity, FALSE: The branch of _branchTopEntity doesn't contain a child with the specified ID.
	*/
	bool IsChildInBranch(const Entity& _branchTopEntity, int _childIDToLookFor)
	{
		return hierarchy.IsChildInBranch(hierarchy.GetNode(_branchTopEntity.id), _childIDToLookFor);
	}


};