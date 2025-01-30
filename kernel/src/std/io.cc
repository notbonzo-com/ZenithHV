#include <io>

namespace io {

	template < typename T >
	void out( uint16_t port, T value ) {
		static_assert( sizeof( T ) == 1 || sizeof( T ) == 2 || sizeof( T ) == 4, "Unsupported size for port output." );
		if constexpr ( sizeof( T ) == 1 ) {
			asm volatile ( "outb %0, %1" : : "a"( value ), "Nd"( port ) );
		} else if constexpr ( sizeof( T ) == 2 ) {
			asm volatile ( "outw %0, %1" : : "a"( value ), "Nd"( port ) );
		} else if constexpr ( sizeof( T ) == 4 ) {
			asm volatile ( "outl %0, %1" : : "a"( value ), "Nd"( port ) );
		}
	}

	template < typename T >
	T in( uint16_t port ) {
		static_assert( sizeof( T ) == 1 || sizeof( T ) == 2 || sizeof( T ) == 4, "Unsupported size for port input." );
		T ret;
		if constexpr ( sizeof( T ) == 1 ) {
			asm volatile ( "inb %1, %0" : "=a"( ret ) : "Nd"( port ) );
		} else if constexpr ( sizeof( T ) == 2 ) {
			asm volatile ( "inw %1, %0" : "=a"( ret ) : "Nd"( port ) );
		} else if constexpr ( sizeof( T ) == 4 ) {
			asm volatile ( "inl %1, %0" : "=a"( ret ) : "Nd"( port ) );
		}
		return ret;
	}

	template < typename T >
	void ins( uint16_t port, void *addr, uint32_t count ) {
		static_assert( sizeof( T ) == 1 || sizeof( T ) == 2 || sizeof( T ) == 4, "Unsupported size for port input." );
		if constexpr ( sizeof( T ) == 1 ) {
			asm volatile ( "rep insb" : "+D"( addr ), "+c"( count ) : "d"( port ) : "memory" );
		} else if constexpr ( sizeof( T ) == 2 ) {
			asm volatile ( "rep insw" : "+D"( addr ), "+c"( count ) : "d"( port ) : "memory" );
		} else if constexpr ( sizeof( T ) == 4 ) {
			asm volatile ( "rep insl" : "+D"( addr ), "+c"( count ) : "d"( port ) : "memory" );
		}
	}

	template < typename T >
	void outs( uint16_t port, const void *addr, uint32_t count ) {
		static_assert( sizeof( T ) == 1 || sizeof( T ) == 2 || sizeof( T ) == 4, "Unsupported size for port output." );
		if constexpr ( sizeof( T ) == 1 ) {
			asm volatile ( "rep outsb" : "+S"( addr ), "+c"( count ) : "d"( port ) );
		} else if constexpr ( sizeof( T ) == 2 ) {
			asm volatile ( "rep outsw" : "+S"( addr ), "+c"( count ) : "d"( port ) );
		} else if constexpr ( sizeof( T ) == 4 ) {
			asm volatile ( "rep outsl" : "+S"( addr ), "+c"( count ) : "d"( port ) );
		}
	}

	template void out< uint8_t >( uint16_t port, uint8_t value );
	template void out< uint16_t >( uint16_t port, uint16_t value );
	template void out< uint32_t >( uint16_t port, uint32_t value );

	template uint8_t in< uint8_t >( uint16_t port );
	template uint16_t in< uint16_t >( uint16_t port );
	template uint32_t in< uint32_t >( uint16_t port );

	template void ins< uint8_t >( uint16_t port, void *addr, uint32_t count );
	template void ins< uint16_t >( uint16_t port, void *addr, uint32_t count );
	template void ins< uint32_t >( uint16_t port, void *addr, uint32_t count );

	template void outs< uint8_t >( uint16_t port, const void *addr, uint32_t count );
	template void outs< uint16_t >( uint16_t port, const void *addr, uint32_t count );
	template void outs< uint32_t >( uint16_t port, const void *addr, uint32_t count );

	void cli() {
		asm volatile ( "cli" );
	}

	void sti() {
		asm volatile ( "sti" );
	}

	void hlt() {
		asm volatile ( "hlt" );
	}

	void pause() {
		asm volatile ("pause");
	}

	bool is_interrupts_enabled() {
		uint64_t rflags;
		asm volatile (
			"pushfq\n\t"
			"popq %0"
			: "=r"( rflags )
			:
			: "memory"
		);
		return ( rflags & 0x200 ) != 0;
	}

	uint64_t read_msr( uint32_t msr ) {
		uint32_t low, high;
		asm volatile ( "rdmsr" : "=a"( low ), "=d"( high ) : "c"( msr ) );
		return ( ( uint64_t ) high << 32 ) | low;
	}

	void write_msr( uint32_t msr, uint64_t value ) {
		uint32_t low = value & 0xFFFFFFFF;
		uint32_t high = value >> 32;
		asm volatile ( "wrmsr" : : "a"( low ), "d"( high ), "c"( msr ) );
	}

	void io_wait() {
		asm volatile ( "outb %%al, $0x80" : : "a"( 0 ) );
	}

	void memory_barrier() {
		asm volatile ( "" ::: "memory" );
	}

	void io_memory_barrier() {
		asm volatile ( "mfence" ::: "memory" );
	}

	void invalidate_cache() {
		asm volatile ( "wbinvd" );
	}

	void flush_cache( const void *addr ) {
		asm volatile ( "clflush (%0)" : : "r"( addr ) );
	}

}
