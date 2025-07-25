template <typename EnumType>
class CInteractionEnumLayer {
public:
	CInteractionEnumLayer(EnumType& stateRef) : state(stateRef) {}

	template<typename... States>
	bool find(States... states) const {
		return ((state == states) || ...);
	}

	template<typename... States>
	bool swap(EnumType reset, States... states) {
		bool found = ((state == states) || ...);
		if (found)
			state = reset;
		return found;
	}

	void set(EnumType newState) {
		state = newState;
	}

	EnumType get() const {
		return state;
	}

private:
	EnumType& state;
};