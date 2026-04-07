Low-Level Design: TCP File Transfer Utility
===========================================

1\. Constants and Data Structures
---------------------------------

To support files up to 16 GB, all size and offset variables use 64-bit unsigned integers (uint64\_t).

### 1.1 Wire Protocol Header

A packed structure ensures binary compatibility across different operating systems#pragma pack(push, 1)struct PacketHeader { uint32\_t magic; // Fixed: 0x54465541 ("TFUA") uint8\_t type; // 0x01: Meta, 0x02: Data, 0x03: EOF/ACK uint32\_t payload\_size; // Size of data following this header uint64\_t sequence\_id; // Incremental ID for tracking/retries uint8\_t checksum\[32\]; // SHA-256 hash of the payload};#pragma pack(pop)1.2 Metadata Payload

Sent at the start of the session to prepare the receiver.

C++

Plain textANTLR4BashCC#CSSCoffeeScriptCMakeDartDjangoDockerEJSErlangGitGoGraphQLGroovyHTMLJavaJavaScriptJSONJSXKotlinLaTeXLessLuaMakefileMarkdownMATLABMarkupObjective-CPerlPHPPowerShell.propertiesProtocol BuffersPythonRRubySass (Sass)Sass (Scss)SchemeSQLShellSwiftSVGTSXTypeScriptWebAssemblyYAMLXML`struct FileMetadata {      char     filename[256];      uint64_t total_size;    // Support for 16 GB+       uint64_t total_chunks;  // total_size / CHUNK_SIZE  };`      

2\. Core Class Interfaces
-------------------------

### 2.1 ISocket (Network Abstraction)

Provides a unified interface for POSIX (Linux/macOS) and Winsock2 (Windows).

*   **bool Connect(string ip, int port)**: Establishes a client connection.
    
*   **bool Listen(int port)**: Sets up a server to accept incoming transfers.
    
*   **int Send(void\* data, size\_t len)**: Reliable data transmission.
    
*   **int Receive(void\* data, size\_t len)**: Blocking read for data chunks.
    

### 2.2 FileHandler (Buffered I/O)

Manages file streams to ensure the utility does not load the entire 16 GB file into RAM.

*   **size\_t ReadChunk(char\* buffer, size\_t size)**: Reads 1 MB from disk into a pre-allocated buffer.
    
*   **bool WriteChunk(const char\* buffer, size\_t size)**: Persists received data to the target path.
    
*   **void Seek(uint64\_t offset)**: Moves the file pointer for resuming or out-of-order writes.
    

### 2.3 IntegrityManager (Security)

Wraps cryptographic functions for integrity validation.

*   **ComputeHash(data, len, out\_hash)**: Generates a SHA-256 signature for a chunk.
    
*   **VerifyHash(data, len, expected\_hash)**: Compares computed hash against the header's checksum.
    

3\. Detailed Logic Flow
-----------------------

### 3.1 Sender Sequence

1.  **Initialize**: Open the source file and calculate the total number of 1 MB chunks.
    
2.  **Handshake**: Construct and send a PacketHeader (Type 0x01) containing the FileMetadata.
    
3.  **Transfer Loop**:
    
    *   Send PacketHeader (Type 0x02) followed immediately by the raw data.
        
    
4.  **Finalize**: Send an EOF packet (Type 0x03) and close the file handle.
    

### 3.2 Receiver Sequence

1.  **Listen**: Wait for a connection and read the FileMetadata.
    
2.  **Pre-allocate**: Check disk space and create a placeholder file of the specified size.
    
3.  **Ingestion Loop**:
    
    *   Read the 45-byte PacketHeader.
        
    *   Allocate/Reuse a 1 MB buffer to read the incoming payload.
        
    
    *   Write valid data to the correct file offset.
        
4.  **Cleanup**: Once the EOF packet is received, sync the file to disk and close the connection.
    

4\. Error Handling Matrix

**Error ConditionReporting MechanismRecovery ActionConnection Lost**Socket recv() returns 0 or -1.Terminate and log "Last Successful Sequence ID".**Checksum Failure**VerifyHash() returns false.Log error; future implementation: Request Retry.**Disk Full**WriteChunk() returns false.Send "NACK" to sender and abort transfer.**Invalid Header**Magic Number mismatch.Immediately drop the connection for security.Resource Management

5\. Resource managment
*   **Memory**: Fixed buffer allocation of 1 MB (plus headers) ensures constant memory usage regardless of file size.
    
*   **CPU**: Hashing is performed per chunk to balance security with throughput.
    
*   **Storage**: Direct-to-disk writing avoids intermediate temporary files.