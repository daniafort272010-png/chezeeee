#pragma once

class GUI {
	ImVec2 size;
	bool initialized = false;
public:
	void initialize( ID3D11Device* device );
	void draw( );
	bool is_initialized( ) const { return initialized; }

	static GUI& get( ) {
		static GUI s{};
		return s;
	}
};